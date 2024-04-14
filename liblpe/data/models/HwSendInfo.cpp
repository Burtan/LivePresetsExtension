/******************************************************************************
/ LivePresetsExtension
/
/ Represents a hardware send
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

#include <liblpe/data/models/HwSendInfo.h>
#include <liblpe/data/models/base/BaseTrackInfo.h>
#include <liblpe/plugins/reaper_plugin_functions.h>
#include <liblpe/data/models/FilterPreset.h>
#include <liblpe/util/util.h>

/**
 * Create a new SendInfo object from a track
 * @param srcGuid the track guid
 * @param sendidx the id of the send
 */
HwSendInfo::HwSendInfo(Filterable* parent, GUID srcGuid, int sendidx) : BaseSendInfo(parent, srcGuid, sendidx) {
    HwSendInfo::saveCurrentState(false);
}

/**
 * Recreate a saved SendInfo from a reaper chunk
 * @param ctx
 */
HwSendInfo::HwSendInfo(Filterable* parent, ProjectStateContext* ctx) : BaseSendInfo(parent) {
    initFromChunk(ctx);
}

/**
 * sendidx has to be defined before calling this function to determine the underlying send. When updating the state
 * sendidx should be matched by output channels when possible
 * @param update
 */
void HwSendInfo::saveCurrentState(bool update) {
    BaseSendInfo::saveCurrentState(update);

    //get track settings
    for (const auto& key : getKeys()) {
        FilterMode filter = update ? mParamInfo.at(key).mFilter : RECALLED;
        auto param = Parameter<double>(&mParamInfo, key, GetTrackSendInfo_Value(getSrcTrack(), 1, mSendIdx, key.data()), filter);
        mParamInfo.insert(key, param);
    }
}

void HwSendInfo::recallSettings() const {
    BaseSendInfo::recallSettings();

    MediaTrack* tr = getSrcTrack();

    if (mSendIdx < 0)
        mSendIdx = CreateTrackSend(tr, nullptr);

    for (const auto& key : getKeys()) {
        if (GetTrackSendInfo_Value(tr, 1, mSendIdx, key.data()) != mParamInfo.at(key).mValue)
            SetTrackSendInfo_Value(tr, 1, mSendIdx, key.data(), mParamInfo.at(key).mValue);
    }
}

bool HwSendInfo::initFromChunkHandler(std::string &key, std::vector<const char *> &params) {
    return BaseSendInfo::initFromChunkHandler(key, params);
}

void HwSendInfo::persistHandler(WDL_FastString &str) const {
    BaseSendInfo::persistHandler(str);
}

/**
 * Helper function to get the MediaTrack* object for the liblpe track
 * @return the MediaTrack*
 */
MediaTrack* HwSendInfo::getSrcTrack() const {
    if (GuidsEqual(mSrcTrackGuid, BaseTrackInfo::MASTER_GUID))
        return GetMasterTrack(nullptr);

    for (int i = 0; i < CountTracks(nullptr); ++i)
    {
        MediaTrack* track = GetTrack(nullptr, i);
        if (GuidsEqual(mSrcTrackGuid, *GetTrackGUID(track)))
            return track;
    }
    return nullptr;
}

std::set<std::string> HwSendInfo::getKeys() const {
    auto set = BaseSendInfo::getKeys();
    std::set<std::string> keys = {

    };
    set.insert(keys.begin(), keys.end());
    return set;
}

char * HwSendInfo::getTreeText() const {
    mName.clear();
    mName.append("Send from ");

    int src = (int) mParamInfo.at(I_SRCCHAN).mValue;

    int srcChStart;
    int srcChCount;

    if (src == -1) {
        mName.append("None");
        srcChCount = 2;
    } else {
        //10 bits are used for the starting source channel
        //shift them to get the channel count
        auto srcCountFlag = src >> 10;
        //turn all other bits but the 10 starting bits off
        auto srcStartFlag = src & 0x000003FF;

        srcChStart = srcStartFlag + 1;
        switch (srcCountFlag) {
            case -1:
                srcChCount = 0;
                break;
            case 0:
                srcChCount = 2;
                break;
            case 1:
                srcChCount = 1;
                break;
            default:
                srcChCount = srcCountFlag * 2;
                break;
        }

        mName.append(std::to_string(srcChStart) + "-" + std::to_string(srcChStart + srcChCount - 1));
    }

    int dst = (int) mParamInfo.at(I_DSTCHAN).mValue;

    int dstChStrt;
    int dstChCnt;

    //10 bits are used for the starting source channel
    //shift them to get the channel count
    auto dstCountFlag = dst >> 10;
    //turn all other bits but the 10 starting bits off
    auto dstStartFlag = dst & 0x000003FF;

    dstChStrt = dstStartFlag + 1;
    switch (dstCountFlag) {
        case -1:
            dstChCnt = 0;
            break;
        case 0:
            dstChCnt = srcChCount;
            break;
        case 1:
            dstChCnt = 1;
            break;
        default:
            dstChCnt = dstCountFlag * 2;
            break;
    }

    mName.append(" to hardware out: " + std::to_string(dstChStrt) + "-" + std::to_string(dstChStrt + dstChCnt - 1));

    std::string newText = getFilterText() + " " + mName;
    newText.copy(mTreeText, newText.length());
    mTreeText[newText.length()] = '\0';
    return mTreeText;
}

std::string HwSendInfo::getChunkId() const {
    return "HWSENDINFO";
}

FilterPreset* HwSendInfo::extractFilterPreset() {
    FilterPreset::ItemIdentifier id{};
    id.data = mParamInfo.at(I_DSTCHAN).mValue;

    auto childs = std::vector<FilterPreset*>();
    childs.push_back(mParamInfo.extractFilterPreset());

    return new FilterPreset(id, SEND, mFilter, childs);
}

bool HwSendInfo::applyFilterPreset(FilterPreset *preset) {
    if (preset->mType == SEND && preset->mId.data == mParamInfo.at(I_DSTCHAN).mValue) {
        mFilter = preset->mFilter;
        for (auto& child : preset->mChilds) {
            if (mParamInfo.applyFilterPreset(child)) {
                continue;
            }
        }
        return true;
    }
    return false;
}