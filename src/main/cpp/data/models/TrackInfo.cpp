/******************************************************************************
/ LivePresetsExtension
/
/ Represents a MediaTrack object with data
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

#include <data/models/TrackInfo.h>
#include <plugins/lpe_ultimate.h>
#include <util/util.h>
#include <plugins/reaper_plugin_functions.h>

TrackInfo::TrackInfo(MediaTrack *track) : mGuid(*GetTrackGUID(track)) {
    TrackInfo::saveCurrentState(false);
}

TrackInfo::TrackInfo(ProjectStateContext* ctx) {
    initFromChunk(ctx);
}

TrackInfo::~TrackInfo() {
    for (auto swSend : mSwSends) {
        delete swSend;
    }
    mSwSends.clear();

    for (auto recFx : mRecFxs) {
        delete recFx;
    }
    mRecFxs.clear();
}

void TrackInfo::saveCurrentState(bool update) {
    BaseTrackInfo::saveCurrentState(update);

    char buffer[256];
    GetTrackName(getMediaTrack(), buffer, sizeof(buffer));
    mName = Parameter<std::string>("NAME", buffer, update ? mName.mFilter : RECALLED);

    //only manage sends and hardware outputs, receives are automatically created by sends
    BaseTrackInfo::saveSwSendState(mSwSends, getMediaTrack(), &mGuid, update);
    BaseTrackInfo::saveHwSendState(mHwSends, getMediaTrack(), &mGuid, update);
    BaseTrackInfo::saveFxState(mFxs, getMediaTrack(), &mGuid, update);
    BaseTrackInfo::saveFxState(mRecFxs, getMediaTrack(), &mGuid, update, true);
}

bool TrackInfo::initFromChunkHandler(std::string& key, std::vector<const char*>& params) {
    if (key == "GUID") {
        stringToGuid(params[0], &mGuid);
        return true;
    }
    if (key == "NAME") {
        mName = Parameter<std::string>("NAME", params[0], (FilterMode) std::stoi(params[1]));
        return true;
    }
    return BaseTrackInfo::initFromChunkHandler(key, params);
}

bool TrackInfo::initFromChunkHandler(std::string &key, ProjectStateContext *ctx) {
    if (key == "RECFXINFO") {
        mRecFxs.push_back(new FxInfo(ctx));
        return true;
    }
    if (key == "SWSENDINFO") {
        mSwSends.push_back(new SwSendInfo(ctx));
        return true;
    }
    return BaseTrackInfo::initFromChunkHandler(key, ctx);
}

void TrackInfo::recallSettings(FilterMode parentFilter) const {
    if (parentFilter == IGNORED || mFilter == IGNORED)
        return;
    BaseTrackInfo::recallSettings(parentFilter);
    FilterMode filter = Merge(parentFilter, mFilter);

    //name can only be recalled by SetTrackStateChunk
/*    if (Merge(filter, mName.mFilter) == RECALLED) {
        char guid[256];
        guidToString(&mGuid, guid);
        std::string chunk = "<TRACK " + std::string(guid) + "\n" + "NAME " + mName.mValue + "\n>";
        //SetTrackState sets empty values to default value
        SetTrackStateChunk(getMediaTrack(), chunk.data(), false);
    }*/

    //prepare lists for matching sends (that can be reused) and nonMatching sends (that have to be removed)
    auto nonMatchingSends = std::set<SwSendInfo*>(mSwSends.begin(), mSwSends.end());
    auto matchingSends = std::vector<SwSendInfo*>(GetTrackNumSends(getMediaTrack(), 0));

    //go through all sends and check for matching destination because you can't change them
    //remove all obsolete sends, then recall sends beginning with those who have a matching send
    //count backwards that removed tracks dont change other tracks index
    for (int i = GetTrackNumSends(getMediaTrack(), 0) - 1; i >= 0; i--) {

        auto dst = (long long) GetTrackSendInfo_Value(getMediaTrack(), 0, i, "P_DESTTRACK");
        auto dstTrackGuid = *GetTrackGUID((MediaTrack*) dst);

        for (SwSendInfo* sendInfo : nonMatchingSends) {
            if (GuidsEqual(sendInfo->mDstTrackGuid, dstTrackGuid)) {
                matchingSends[i] = sendInfo;
                nonMatchingSends.erase(sendInfo);
                goto matched;
            }
        }
        RemoveTrackSend(getMediaTrack(), 0, i);
        matchingSends.erase(matchingSends.begin() + i);
        matched:;
    }

    int index = 0;
    for (const SwSendInfo* send : matchingSends) {
        send->mSendIdx = index;
        send->recallSettings(parentFilter);
        index++;
    }
    for (const SwSendInfo* send : nonMatchingSends) {
        send->mSendIdx = -1;
        send->recallSettings(parentFilter);
    }

    //assign input plugin settings
    auto muted = (bool) mParamInfo.at(B_MUTE).mValue;
    if (!muted) {
        for (const auto recFxInfo : mRecFxs) {
            recFxInfo->recallSettings(filter);
        }
    }
}

void TrackInfo::persistHandler(WDL_FastString &str) const {
    BaseTrackInfo::persistHandler(str);

    char dest[64];
    guidToString(&mGuid, dest);
    str.AppendFormatted(4096, "GUID %s\n", dest);
    str.AppendFormatted(4096, "NAME \"%s\" %u\n", mName.mValue.data(), mName.mFilter);

    for (const auto recFx : mRecFxs) {
        recFx->persist(str);
    }

    for (const auto send : mSwSends) {
        send->persist(str);
    }
}

MediaTrack* TrackInfo::getMediaTrack() const {
    for (int i = 0; i < CountTracks(nullptr); ++i) {
        MediaTrack* track = GetTrack(nullptr, i);
        if (GuidsEqual(mGuid, *GetTrackGUID(track)))
            return track;
    }
    return nullptr;
}

std::set<std::string> TrackInfo::getKeys() const {
    auto set = BaseTrackInfo::getKeys();
    std::set<std::string> keys = {
            B_PHASE,
            I_RECARM,
            I_RECINPUT,
            I_RECMODE,
            I_RECMON,
            I_RECMONITEMS,
            I_FOLDERDEPTH,
            I_FOLDERCOMPACT,
            I_PANMODE,
            D_PANLAW,
            B_SHOWINMIXER,
            B_SHOWINTCP,
            B_MAINSEND,
            C_MAINSEND_OFFS,
            B_FREEMODE,
    };

    set.insert(keys.begin(),keys.end());
    return set;
}

char* TrackInfo::getTreeText() const {
    std::string newText = getFilterText() + " Track: " + mName.mValue;
    newText.copy(mTreeText, newText.length());
    mTreeText[newText.length()] = '\0';
    return mTreeText;
}

std::string TrackInfo::getChunkId() const {
    return "TRACKINFO";
}

FilterPreset* TrackInfo::extractFilterPreset() {
    return TrackInfo_ExtractFilterPreset(this);
}

bool TrackInfo::applyFilterPreset(FilterPreset *preset) {
   return TrackInfo_ApplyFilterPreset(this, preset);
}