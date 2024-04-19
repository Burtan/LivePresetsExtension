/******************************************************************************
/ LivePresetsExtension
/
/ TreeAdapter for showing preset's filter options
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

#include <liblpe/ui/base/TreeView.h>
#include <liblpe/ui/LivePresetsTreeAdapter.h>
#include <liblpe/data/models/TrackInfo.h>

LivePresetsTreeAdapter::LivePresetsTreeAdapter(LivePreset *preset) : mPreset(preset) {}

void LivePresetsTreeAdapter::onAction(HWND hwnd, HTREEITEM hItem) {
    TVITEM qItem{};
    qItem.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_TEXT;
    qItem.hItem = hItem;
    TreeView_GetItem(hwnd, &qItem);
    ItemData data = mData[qItem.lParam];
    switch (data.type) {
        case TYPE::MASTERTRACK: {
            auto* masterTrack = (MasterTrackInfo*) data.lParam;
            masterTrack->shuffleFilter();
            qItem.pszText = masterTrack->getTreeText();
            break;
        }
        case TYPE::TRACK: {
            auto* track = (TrackInfo*) data.lParam;
            track->shuffleFilter();
            qItem.pszText = track->getTreeText();
            break;
        }
        case TYPE::PARAMS: {
            auto* params = (ParameterInfo*) data.lParam;
            params->shuffleFilter();
            qItem.pszText = params->getTreeText();
            break;
        }
        case TYPE::FX: {
            auto* fx = (FxInfo*) data.lParam;
            fx->shuffleFilter();
            qItem.pszText = fx->getTreeText();
            break;
        }
        case TYPE::SEND: {
            auto* send = (BaseSendInfo*) data.lParam;
            send->shuffleFilter();
            qItem.pszText = send->getTreeText();
            break;
        }
        case TYPE::PARAM: {
            auto* param = (Parameter<double>*) data.lParam;
            param->shuffleFilter(true);
            qItem.pszText = param->getTreeText();
            break;
        }
        case TYPE::CTRL:
        case TYPE::LIVEPRESET:
        case TYPE::NOTHING:
            //should not happen
            break;
    }
    TreeView_SetItem(hwnd, &qItem);
}

std::vector<TVITEM> LivePresetsTreeAdapter::getChilds(TVITEM* parent) {

    std::vector<TVITEM> childs;
    if (!parent) {
        mData.clear();
        // root node, show tracks
        {
            TVITEM child;
            child.mask = TVIF_TEXT| TVIF_PARAM | TVIF_STATE;

            child.pszText = mPreset->mMasterTrack->getTreeText();
            child.cchTextMax = sizeof(mPreset->mMasterTrack->getTreeText());

            auto lparam = (LPARAM) mPreset->mMasterTrack;
            mData[lparam] = {TYPE::MASTERTRACK, lparam, TYPE::LIVEPRESET};
            child.lParam = lparam;

            child.state = TVIS_EXPANDED;
            childs.push_back(child);
        }

        for (auto track : mPreset->mTracks) {
            TVITEM child;
            child.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;

            child.pszText = track->getTreeText();
            child.cchTextMax = sizeof(track->getTreeText());

            auto lparam = (LPARAM) track;
            mData[lparam] = {TYPE::TRACK, lparam, TYPE::LIVEPRESET};
            child.lParam = lparam;

            child.state = TVIS_EXPANDED;
            childs.push_back(child);
        }
    } else {
        ItemData data = mData[parent->lParam];
        switch (data.type) {
            case TYPE::MASTERTRACK: {
                addChildsForMasterTrack((MasterTrackInfo*) data.lParam, &childs);
                break;
            }
            case TYPE::TRACK: {
                addChildsForTrack((TrackInfo*) data.lParam, &childs);
                break;
            }
            case TYPE::PARAMS: {
                if (data.parentType != FX) {
                    addChildsForParams((ParameterInfo*) data.lParam, &childs);
                }
                break;
            }
            case TYPE::FX: {
                addChildsForFx((FxInfo*) data.lParam, &childs);
                break;
            }
            case TYPE::SEND: {
                addChildsForSend((BaseSendInfo*) data.lParam, &childs);
                break;
            }
            case TYPE::NOTHING:
            case TYPE::CTRL:
            case TYPE::PARAM:
            case TYPE::LIVEPRESET:
                break;
        }
    }

    return childs;
}

void LivePresetsTreeAdapter::addChildsForTrack(TrackInfo* item, std::vector<TVITEM>* childs) {
    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = item->mName.getTreeText();
        child.cchTextMax = sizeof(item->mName.getTreeText());

        auto lparam = (LPARAM) &item->mName;
        mData[lparam] = {TYPE::PARAM, lparam, TYPE::TRACK};
        child.lParam = lparam;

        childs->push_back(child);
    }

    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = item->mParamInfo.getTreeText();
        child.cchTextMax = sizeof(item->mParamInfo.getTreeText());

        auto lparam = (LPARAM) &item->mParamInfo;
        mData[lparam] = {TYPE::PARAMS, lparam, TYPE::TRACK};
        child.lParam = lparam;

        childs->push_back(child);
    }

    for (auto& fxInfo : item->mRecFxs) {
        TVITEM child;
        child.mask = TVIF_TEXT| TVIF_PARAM;

        child.pszText = fxInfo->getTreeText();
        child.cchTextMax = sizeof(fxInfo->getTreeText());

        auto lparam = (LPARAM) fxInfo;
        mData[lparam] = {TYPE::FX, lparam, TYPE::TRACK};
        child.lParam = lparam;

        childs->push_back(child);
    }

    for (auto& fxInfo : item->mFxs) {
        TVITEM child;
        child.mask = TVIF_TEXT| TVIF_PARAM;

        child.pszText = fxInfo->getTreeText();
        child.cchTextMax = sizeof(fxInfo->getTreeText());

        auto lparam = (LPARAM) fxInfo;
        mData[lparam] = {TYPE::FX, lparam, TYPE::TRACK};
        child.lParam = lparam;

        childs->push_back(child);
    }

    for (auto sendInfo : item->mSwSends) {
        addSendInfoChild(sendInfo, childs);
    }

    for (auto sendInfo : item->mHwSends) {
        addSendInfoChild(sendInfo, childs);
    }
}

void LivePresetsTreeAdapter::addChildsForMasterTrack(MasterTrackInfo* item, std::vector<TVITEM>* childs) {
    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = item->mParamInfo.getTreeText();
        child.cchTextMax = sizeof(item->mParamInfo.getTreeText());

        auto lparam = (LPARAM) &item->mParamInfo;
        mData[lparam] = {TYPE::PARAMS, lparam, TYPE::MASTERTRACK};
        child.lParam = lparam;

        childs->push_back(child);
    }

    for (auto fxInfo : item->mFxs) {
        TVITEM child;
        child.mask = TVIF_TEXT| TVIF_PARAM;

        child.pszText = fxInfo->getTreeText();
        child.cchTextMax = sizeof(fxInfo->getTreeText());

        auto lparam = (LPARAM) fxInfo;
        mData[lparam] = {TYPE::FX, lparam, TYPE::MASTERTRACK};
        child.lParam = lparam;

        childs->push_back(child);
    }

    for (auto sendInfo : item->mHwSends) {
        addSendInfoChild(sendInfo, childs);
    }
}

void LivePresetsTreeAdapter::addChildsForParams(ParameterInfo* item, std::vector<TVITEM>* childs) {
    for (auto& key : item->getKeys()) {
        const auto& param = item->at(key);
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = param.getTreeText();
        child.cchTextMax = sizeof(param.getTreeText());

        auto lparam = (LPARAM) &param;
        mData[lparam] = {TYPE::PARAM, lparam, TYPE::PARAMS};
        child.lParam = lparam;

        childs->push_back(child);
    }
}

void LivePresetsTreeAdapter::addChildsForFx(FxInfo* item, std::vector<TVITEM>* childs) {

    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = item->mIndex.getTreeText();
        child.cchTextMax = sizeof(item->mIndex.getTreeText());

        auto lparam = (LPARAM) &item->mIndex;

        mData[lparam] = {TYPE::PARAM, lparam, TYPE::FX};
        child.lParam = lparam;

        childs->push_back(child);
    }
    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = item->mEnabled.getTreeText();
        child.cchTextMax = sizeof(item->mEnabled.getTreeText());

        auto lparam = (LPARAM) &item->mEnabled;
        mData[lparam] = {TYPE::PARAM, lparam, TYPE::FX};
        child.lParam = lparam;

        childs->push_back(child);
    }
    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = item->mPresetName.getTreeText();
        child.cchTextMax = sizeof(item->mPresetName.getTreeText());

        auto lparam = (LPARAM) &item->mPresetName;
        mData[lparam] = {TYPE::PARAM, lparam, TYPE::FX};
        child.lParam = lparam;

        childs->push_back(child);
    }
    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = item->mParamInfo.getTreeText();
        child.cchTextMax = sizeof(item->mParamInfo.getTreeText());

        auto lparam = (LPARAM) &item->mParamInfo;
        mData[lparam] = {TYPE::PARAMS, lparam, TYPE::FX};
        child.lParam = lparam;

        childs->push_back(child);
    }
}

void LivePresetsTreeAdapter::addChildsForSend(BaseSendInfo* item, std::vector<TVITEM>* childs) {
    TVITEM child;
    child.mask = TVIF_TEXT | TVIF_PARAM;

    child.pszText = item->mParamInfo.getTreeText();
    child.cchTextMax = sizeof(item->mParamInfo.getTreeText());

    auto lparam = (LPARAM) &item->mParamInfo;
    mData[lparam] = {TYPE::PARAMS, lparam, TYPE::SEND};
    child.lParam = lparam;

    childs->push_back(child);
}

void LivePresetsTreeAdapter::addSendInfoChild(BaseSendInfo *item, std::vector<TVITEM>* childs) {
    TVITEM child;
    child.mask = TVIF_TEXT | TVIF_PARAM;

    child.pszText = item->getTreeText();
    child.cchTextMax = sizeof(item->getTreeText());

    auto lparam = (LPARAM) item;
    mData[lparam] = {TYPE::SEND, lparam, TYPE::SEND};
    child.lParam = lparam;

    childs->push_back(child);
}

