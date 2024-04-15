/******************************************************************************
/ LivePresetsExtension
/
/ Bass class for TrackInfo and MasterTrackInfo
/
/ Copyright (c) 2020 and later Dr. med. Frederik Bertling
/
/
/ Permission is hereby granted, free of charge, to any person obtaining a copy
/ of this software and associated documentation files (the "Software"), to deal
/ in the Software without restriction, including without limitation the rights to
/ use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
/ of the Software, and to permit persons to whom the Software is furnished to
/ do so, subject to the following conditions:
/
/ The above copyright notice and this permission notice shall be included in all
/ copies or substantial portions of the Software.
/
/ THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
/ EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
/ OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
/ NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
/ HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
/ WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/ FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
/ OTHER DEALINGS IN THE SOFTWARE.
/
******************************************************************************/

#include <liblpe/data/models/base/BaseTrackInfo.h>
#include <liblpe/data/models/TrackInfo.h>
#include <liblpe/data/models/FxInfo.h>
#include <liblpe/data/models/HwSendInfo.h>
#include <liblpe/data/models/SwSendInfo.h>
#include <reaper_plugin_functions.h>
#include <liblpe/LivePresetsExtension.h>
#include <liblpe/util/util.h>

const GUID BaseTrackInfo::MASTER_GUID = GUID{0, 0, 0, 0};

BaseTrackInfo::BaseTrackInfo(Filterable *parent) : BaseInfo(parent) {}

void BaseTrackInfo::saveCurrentState(bool update) {

    //get track settings
    for (const auto& key : getKeys()) {
        double val = GetMediaTrackInfo_Value(getMediaTrack(), key.data());

#ifndef _MACOS
        //mac scales different to linux and win on 4k screens, recall half the height and save double the height
        if (key == I_HEIGHTOVERRIDE) {
            val *= 2;
        }
#endif

        auto param = Parameter<double>(&mParamInfo, key, val, update ? mParamInfo.at(key).mFilter : RECALLED);
        mParamInfo.insert(key, param);
    }
}

bool BaseTrackInfo::initFromChunkHandler(std::string& key, ProjectStateContext* ctx) {
    if (BaseInfo::initFromChunkHandler(key, ctx))
        return true;

    if (key == "FXINFO") {
        mFxs.push_back(new FxInfo(this, ctx));
        return true;
    }
    if (key == "HWSENDINFO") {
        mHwSends.push_back(new HwSendInfo(this, ctx));
        return true;
    }
    return false;
}

bool BaseTrackInfo::initFromChunkHandler(std::string& key, std::vector<const char*>& params) {
    if (BaseInfo::initFromChunkHandler(key, params))
        return true;

    mParamInfo.insert(key, Parameter<double>(&mParamInfo, key, std::stod(params[0]), (FilterMode) std::stoi(params[1])));
    return true;
}

void BaseTrackInfo::recallSettings() const {
    if (isFilteredInChain())
        return;

    //assign track settings, only change when needed
    //recall parameters
    for (const auto& key : getKeys()) {
        if (mParamInfo.at(key).isFilteredInChain())
            continue;

        double value = mParamInfo.at(key).mValue;

#ifndef _MACOS
        //mac scales different to linux and win on 4k screens, recall half the height and save double the height
        if (key == I_HEIGHTOVERRIDE) {
            value = value / 2;
        }
#endif

        //override show tracks when global option is active
        if (key == B_SHOWINTCP && g_lpe->mModel->mIsHideMutedTracks && mParamInfo.at(B_MUTE).mValue == 1) {
            value = 0;
        }

        if (GetMediaTrackInfo_Value(getMediaTrack(), key.data()) != value)
            SetMediaTrackInfo_Value(getMediaTrack(), key.data(), value);
    }

    recallHwSends();

    auto muted = (bool) mParamInfo.at(B_MUTE).mValue;
    if (!muted) {
        for (auto *const fxInfo : mFxs) {
            fxInfo->recallSettings();
        }
    }
}

/**
 * All hw sends can be adepted to fit specific parameters so only make sure the count is matching
 */
void BaseTrackInfo::recallHwSends() const {
    MediaTrack* track = getMediaTrack();

    while (GetTrackNumSends(track, 1) > mHwSends.size())
        RemoveTrackSend(track, 1, 0);

    while (GetTrackNumSends(track, 1) < mHwSends.size())
        CreateTrackSend(track, nullptr);

    int index = 0;
    for (auto *const send : mHwSends) {
        send->mSendIdx = index;
        send->recallSettings();
        index++;
    }
}

void BaseTrackInfo::persistHandler(WDL_FastString &str) const {
    BaseInfo::persistHandler(str);

    for (const auto& send : mHwSends) {
        send->persist(str);
    }
    for (const auto& fx : mFxs) {
        fx->persist(str);
    }
}

std::set<std::string> BaseTrackInfo::getKeys() const {
    auto set = BaseInfo::getKeys();
    std::set<std::string> keys = {
            B_MUTE,
            I_SOLO,
            I_FXEN,
            I_AUTOMODE,
            I_NCHAN,
            I_SELECTED,
            I_MIDIHWOUT,
            I_PERFFLAGS,
            I_CUSTOMCOLOR,
            I_HEIGHTOVERRIDE,
            B_HEIGHTLOCK,
            D_VOL,
            D_PAN,
            D_WIDTH,
            D_DUALPANL,
            D_DUALPANR,
            I_PANMODE,
            D_PANLAW,
            C_BEATATTACHMODE,
            F_MCP_FXSEND_SCALE,
            F_MCP_SENDRGN_SCALE
    };
    set.insert(keys.begin(), keys.end());
    return set;
}

void BaseTrackInfo::saveHwSendState(Filterable *parent, std::vector<HwSendInfo *> &hwSends, MediaTrack *track,
                                    const GUID *guid, bool update) {
    if (update) {
        //search for matching output channels and resave these
        auto savedHwSends = std::vector<HwSendInfo*>(hwSends);

        //first int is the dst channel, second int is the send index
        auto currentHwSends = std::vector<std::pair<int, int>>();
        for (int i = 0; i < GetTrackNumSends(track, 1); i++) {
            auto val = std::pair<int, int>(GetTrackSendInfo_Value(track, 1, i, I_DSTCHAN), i);
            currentHwSends.push_back(val);
        }

        bool (*cmp)(HwSendInfo*&, std::pair<int, int>&) = [](HwSendInfo*& a, std::pair<int, int>& b) -> bool {
            return a->mParamInfo.at(I_DSTCHAN).mValue == b.first;
        };

        auto hwSendsDelete = std::vector<HwSendInfo*>();
        auto hwSendsNew = std::vector<std::pair<int, int>>();
        auto hwSendsUpdate = std::vector<std::pair<HwSendInfo*, std::pair<int, int>>>();
        compareVectors(savedHwSends, currentHwSends, hwSendsDelete, hwSendsNew, hwSendsUpdate, *cmp);

        for (auto& hwSendUpdate : hwSendsUpdate) {
            hwSendUpdate.first->mSendIdx = hwSendUpdate.second.second;
            hwSendUpdate.first->saveCurrentState(update);
        }

        for (HwSendInfo* hwSendDelete : hwSendsDelete) {
            int index = 0;
            for (int i = 0; i < hwSends.size(); i++) {
                if (hwSends[i] == hwSendDelete) {
                    index = i;
                    break;
                }
            }
            hwSends.erase(hwSends.begin() + index);
        }

        for (std::pair<int, int>& hwSendNew : hwSendsNew) {
            auto *info = new HwSendInfo(parent, *guid, hwSendNew.second);
            hwSends.push_back(info);
        }
    } else {
        for (int i = 0; i < GetTrackNumSends(track, 1); i++) {
            auto *info = new HwSendInfo(parent, *guid, i);
            hwSends.push_back(info);
        }
    }
}

void BaseTrackInfo::saveSwSendState(Filterable *parent, std::vector<SwSendInfo *> &swSends, MediaTrack *track,
                                    const GUID *guid, bool update) {
    if (update) {
        //search for matching output channels and resave these
        auto savedSwSends = std::vector<SwSendInfo*>(swSends);

        auto currentSwSends = std::vector<std::pair<const GUID*, int>>();
        for (int i = 0; i < GetTrackNumSends(track, 0); i++) {
            auto dst = (long long) GetTrackSendInfo_Value(track, 0, i, P_DESTTRACK);
            auto *dstTrack = (MediaTrack*) dst;
            auto val = std::pair<const GUID*, int>(GetTrackGUID(dstTrack), i);
            currentSwSends.push_back(val);
        }

        bool (*cmp)(SwSendInfo*&, std::pair<const GUID*, int>&) =
                [](SwSendInfo*& a, std::pair<const GUID*, int>& b) -> bool {
            return GuidsEqual(a->mDstTrackGuid, *b.first);
        };

        auto swSendsDelete = std::vector<SwSendInfo*>();
        auto swSendsNew = std::vector<std::pair<const GUID*, int>>();
        auto swSendsUpdate = std::vector<std::pair<SwSendInfo*, std::pair<const GUID*, int>>>();
        compareVectors(savedSwSends, currentSwSends, swSendsDelete, swSendsNew, swSendsUpdate, *cmp);

        for (auto& swSendUpdate : swSendsUpdate) {
            swSendUpdate.first->mSendIdx = swSendUpdate.second.second;
            swSendUpdate.first->saveCurrentState(update);
        }

        for (SwSendInfo* swSendDelete : swSendsDelete) {
            int index = 0;
            for (int i = 0; i < swSends.size(); i++) {
                if (swSends[i] == swSendDelete) {
                    index = i;
                    break;
                }
            }
            swSends.erase(swSends.begin() + index);
        }

        for (auto& swSendNew : swSendsNew) {
            auto *info = new SwSendInfo(parent, *guid, swSendNew.second);
            swSends.push_back(info);
        }
    } else {
        for (int i = 0; i < GetTrackNumSends(track, 0); i++) {
            auto *info = new SwSendInfo(parent, *guid, i);
            swSends.push_back(info);
        }
    }
}

void BaseTrackInfo::saveFxState(Filterable *parent, std::vector<FxInfo *> &fxs, MediaTrack *track, const GUID *guid,
                                bool update, bool rec) {
    if (update) {
        auto savedFxs = std::vector<FxInfo*>(fxs);

        auto currentFxs = std::vector<const GUID*>();
        if (rec) {
            for (int i = FxInfo::RECFX_INDEX_FACTOR; i < FxInfo::RECFX_INDEX_FACTOR + TrackFX_GetRecCount(track); i++) {
                currentFxs.push_back(TrackFX_GetFXGUID(track, i));
            }
        } else {
            for (int i = 0; i < TrackFX_GetCount(track); i++) {
                currentFxs.push_back(TrackFX_GetFXGUID(track, i));
            }
        }

        bool (*cmp)(FxInfo*&, const GUID*&) = [](FxInfo*& a, const GUID*& b) -> bool {
            return GuidsEqual(a->mGuid, *b);
        };

        auto fxsDelete = std::vector<FxInfo*>();
        auto fxsNew = std::vector<const GUID*>();
        auto fxsUpdate = std::vector<std::pair<FxInfo*, const GUID*>>();
        compareVectors(savedFxs, currentFxs, fxsDelete, fxsNew, fxsUpdate, *cmp);

        for (auto& fxUpdate : fxsUpdate) {
            fxUpdate.first->saveCurrentState(update);
        }

        for (FxInfo* fxDelete : fxsDelete) {
            int index = 0;
            for (int i = 0; i < fxs.size(); i++) {
                if (fxs[i] == fxDelete) {
                    index = i;
                    break;
                }
            }
            fxs.erase(fxs.begin() + index);
        }

        for (const GUID* fxNew : fxsNew) {
            auto *info = new FxInfo(parent, *guid, *fxNew);
            fxs.push_back(info);
        }
    } else {
        if (rec) {
            for (int i = FxInfo::RECFX_INDEX_FACTOR; i < FxInfo::RECFX_INDEX_FACTOR + TrackFX_GetRecCount(track); i++) {
                auto fxGuid = *TrackFX_GetFXGUID(track, i);
                auto *info = new FxInfo(parent, *guid, fxGuid);
                fxs.push_back(info);
            }
        } else {
            for (int i = 0; i < TrackFX_GetCount(track); i++) {
                auto fxGuid = *TrackFX_GetFXGUID(track, i);
                auto *info = new FxInfo(parent, *guid, fxGuid);
                fxs.push_back(info);
            }
        }

    }
}

BaseTrackInfo::~BaseTrackInfo() {
    for (auto *hwSend : mHwSends) {
        delete hwSend;
    }
    mHwSends.clear();

    for (auto *fx : mFxs) {
        delete fx;
    }
    mFxs.clear();
}