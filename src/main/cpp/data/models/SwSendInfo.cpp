/******************************************************************************
/ LivePresetsExtension
/
/ Represents a send object, respective receives are ignored as they are implicite with the sends
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

#include <data/models/SwSendInfo.h>
#include <plugins/reaper_plugin_functions.h>
#include <util/util.h>
#include <data/models/FilterPreset.h>
#include <data/models/base/BaseTrackInfo.h>

/**
 * Create a new SendInfo object from a track
 * @param srcGuid the track guid
 * @param sendidx the id of the send
 */
SwSendInfo::SwSendInfo(Filterable* parent, GUID srcGuid, int sendidx) : BaseSendInfo(parent, srcGuid, sendidx) {
    SwSendInfo::saveCurrentState(false);
}

/**
 * Recreate a saved SendInfo from a reaper chunk
 * @param ctx
 */
SwSendInfo::SwSendInfo(Filterable* parent, ProjectStateContext* ctx) : BaseSendInfo(parent) {
    initFromChunk(ctx);
}

void SwSendInfo::saveCurrentState(bool update) {
    BaseSendInfo::saveCurrentState(update);

    auto dst = (long long) GetTrackSendInfo_Value(getSrcTrack(), 0, mSendIdx, P_DESTTRACK);
    auto* dstTrack = (MediaTrack*) dst;
    mDstTrackGuid = *GetTrackGUID(dstTrack);

    //get track settings
    for (const auto& key : getKeys()) {
        FilterMode filter = update ? mParamInfo.at(key).mFilter : RECALLED;
        auto param = Parameter<double>(&mParamInfo, key, GetTrackSendInfo_Value(getSrcTrack(), 0, mSendIdx, key.data()), filter);
        mParamInfo.insert(key, param);
    }
}

/**
* sendidx has to be assigned before calling this function, normally done by parent TrackInfo
* Recalls the settings for an existing send or if sendidx < 0 creates a new send
* @param sendidx >= 0 means there is already a fitting send whose attributes can be changed; < 0 means a new send has
* to be created
*/
void SwSendInfo::recallSettings(FilterMode filter) const {
    BaseSendInfo::recallSettings(filter);

    MediaTrack* tr = getSrcTrack();

    if (mSendIdx < 0)
        mSendIdx = CreateTrackSend(getSrcTrack(), getDstTrack());

    for (const auto& key : getKeys()) {
        if (GetTrackSendInfo_Value(tr, 0, mSendIdx, key.data()) != mParamInfo.at(key).mValue)
            SetTrackSendInfo_Value(tr, 0, mSendIdx, key.data(), mParamInfo.at(key).mValue);
    }
}

bool SwSendInfo::initFromChunkHandler(std::string &key, std::vector<const char *> &params) {
    if (key == "DSTGUID") {
        stringToGuid(params[0], &mDstTrackGuid);
        return true;
    }
    return BaseSendInfo::initFromChunkHandler(key, params);
}

void SwSendInfo::persistHandler(WDL_FastString &str) const {
    BaseSendInfo::persistHandler(str);

    char dst[64];
    guidToString(&mDstTrackGuid, dst);
    str.AppendFormatted(4096, "DSTGUID %s\n", dst);
}

/**
 * Helper function to get the MediaTrack* object for the dst track
 * @return the MediaTrack*
 */
MediaTrack* SwSendInfo::getDstTrack() const {
    if (GuidsEqual(mDstTrackGuid, BaseTrackInfo::MASTER_GUID))
        return GetMasterTrack(nullptr);

    for (int i = 0; i < CountTracks(nullptr); ++i) {
        MediaTrack* track = GetTrack(nullptr, i);
        if (GuidsEqual(mDstTrackGuid, *GetTrackGUID(track)))
            return track;
    }
    return nullptr;
}

/**
 * Helper function to get the MediaTrack* object for the src track
 * @return the MediaTrack*
 */
MediaTrack* SwSendInfo::getSrcTrack() const {
    for (int i = 0; i < CountTracks(nullptr); ++i) {
        MediaTrack* track = GetTrack(nullptr, i);
        if (GuidsEqual(mSrcTrackGuid, *GetTrackGUID(track)))
            return track;
    }
    return nullptr;
}

std::set<std::string> SwSendInfo::getKeys() const {
    auto set = BaseSendInfo::getKeys();
    std::set<std::string> keys = {

    };
    set.insert(keys.begin(), keys.end());
    return set;
}

char * SwSendInfo::getTreeText() const {
    mName.clear();
    //update the name used to show in the filter tree view

    char buf[256];
    GetTrackName(getDstTrack(), buf, sizeof(buf));
    mName.append("Send to " + std::string(buf));

    std::string newText = getFilterText() + " " + mName;
    newText.copy(mTreeText, newText.length());
    mTreeText[newText.length()] = '\0';
    return mTreeText;
}

std::string SwSendInfo::getChunkId() const {
    return "SWSENDINFO";
}

FilterPreset* SwSendInfo::extractFilterPreset() {
    FilterPreset::ItemIdentifier id{};
    id.guid = mDstTrackGuid;

    auto childs = std::vector<FilterPreset*>();
    childs.push_back(mParamInfo.extractFilterPreset());

    return new FilterPreset(id, SEND, mFilter, childs);
}

bool SwSendInfo::applyFilterPreset(FilterPreset *preset) {
    if (preset->mType == SEND && GuidsEqual(preset->mId.guid, mDstTrackGuid)) {
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