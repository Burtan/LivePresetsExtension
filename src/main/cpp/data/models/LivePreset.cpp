/******************************************************************************
/ LivePresetsExtension
/
/ Represents a saved state that can be recalled
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

#include <data/models/LivePreset.h>
#include <util/util.h>
#include <data/models/FilterPreset.h>
#include <LivePresetsExtension.h>
#include <functional>

LivePreset::LivePreset(std::string name, std::string description) : BaseInfo(nullptr), mName(std::move(name)),
        mDescription(std::move(description)) {
    genGuid(&mGuid);
    LivePreset::saveCurrentState(false);

    //make sure that new presets get a recall id assigned
    mRecallId = g_lpe->mModel.getRecallIdForPreset(this);

    if (mRecallCmdId == 0) {
        createRecallAction();
    }
}

LivePreset::LivePreset(ProjectStateContext *ctx, BaseCommand::CommandID recallCmdId) : BaseInfo(nullptr),
        mRecallCmdId(recallCmdId) {
    initFromChunk(ctx);

    if (mRecallCmdId == 0) {
        createRecallAction();
    }
}

LivePreset::~LivePreset() {
    for (auto track : mTracks) {
        delete track;
    }
    mTracks.clear();
    mControlInfos.clear();

    delete mMasterTrack;
}

/**
 * Move assignment for LivePreset
 */
LivePreset& LivePreset::operator=(LivePreset&& other) noexcept {
    //reassign all rvalues of rvalue reference other
    mName = other.mName;
    mGuid = other.mGuid;
    mParamInfo = other.mParamInfo;
    mFilter = other.mFilter;
    mDate = other.mDate;
    mDescription = other.mDescription;
    mRecallId = other.mRecallId;
    mMasterTrack = other.mMasterTrack;
    mTracks = other.mTracks;
    mControlInfos = other.mControlInfos;
    mRecallCmdId = other.mRecallCmdId;

    //make all pointers null and create empty containers for now empty instance other that its destruction does not
    //affect this instance
    mTracks = std::vector<TrackInfo*>();
    mControlInfos = std::vector<std::shared_ptr<ControlInfo>>();
    mMasterTrack = nullptr;
    mRecallCmdId = 0;

    return *this;
}

/**
 * Saves the current state as a preset
 * @param update true when a presets get updated, false when it is new
 */
void LivePreset::saveCurrentState(bool update) {
    if (update) {
        mDate = time(nullptr);

        //MasterTrack
        mMasterTrack->saveCurrentState(update);

        //TODO update assignments

        //Normal tracks
        auto savedTracks = std::vector<TrackInfo*>(mTracks);
        auto currentTracks = std::vector<const GUID*>();
        for (int i = 0; i < GetNumTracks(); i++) {
            currentTracks.push_back(GetTrackGUID(GetTrack(nullptr, i)));
        }

        auto tracksDelete = std::vector<TrackInfo*>();
        auto tracksNew = std::vector<const GUID*>();
        auto tracksUpdated = std::vector<std::pair<TrackInfo*, const GUID*>>();

        //tracksUpdate are all tracks that remain after tracksDelete are removed from mTracks
        // and before tracksNew are added
        bool (*cmp)(TrackInfo*&, const GUID*&) = [](TrackInfo*& a, const GUID*& b) -> bool {
            return GuidsEqual(a->mGuid, *b);
        };

        compareVectors(savedTracks, currentTracks, tracksDelete, tracksNew, tracksUpdated, *cmp);

        for (auto& trackUpdate : tracksUpdated) {
            trackUpdate.first->saveCurrentState(update);
        }

        for (TrackInfo* trackDelete : tracksDelete) {
            int index = 0;
            for (int i = 0; i < mTracks.size(); i++) {
                if (mTracks[i] == trackDelete) {
                    index = i;
                    break;
                }
            }
            mTracks.erase(mTracks.begin() + index);
        }

        for (const GUID* trackNew : tracksNew) {
            auto* info = new TrackInfo(nullptr, GetTrackByGUID(*trackNew));
            mTracks.push_back(info);
        }

    } else {
        mMasterTrack = new MasterTrackInfo(nullptr);
        for (int i = 0; i < GetNumTracks(); i++) {
            auto* info = new TrackInfo(nullptr, GetTrack(nullptr, i));
            mTracks.push_back(info);
        }
        //TODO save assignments
    }

}

/**
 * Creates a reaper action which can bind to Hotkeys/midi/osc to recall this LivePreset
 */
void LivePreset::createRecallAction() {
    auto name = WDL_FastString();
    char dest[64];
    guidToString(&mGuid, dest);

    name.AppendFormatted(4096, "LPE_RECALLPRESET_%s", dest);

    auto desc = WDL_FastString();
    desc.AppendFormatted(4096, "LPE - Recall preset: %s", mName.data());

    using namespace std::placeholders;
    //transcode guid as 4 int to make it fit to the ActionCommand function
    int ints[4];
    GuidToInts(mGuid, ints);

    mRecallCmdId = g_lpe->mActions.add(new ActionCommand(
            name.Get(),
            desc.Get(),
            std::bind(&LPE::recallPresetByGuid, g_lpe.get(), ints[0], ints[1], ints[2], (HWND) ((long) ints[3]))
    ));
}

void LivePreset::persistHandler(WDL_FastString& str) const {
    BaseInfo::persistHandler(str);

    char dest[64];
    guidToString(&mGuid, dest);
    str.AppendFormatted(4096, "GUID %s\n", dest);

    str.AppendFormatted(4096, "NAME \"%s\"\n", mName.data());
    str.AppendFormatted(4096, "DESC \"%s\"\n", mDescription.data());
    str.AppendFormatted(4096, "DATE %li\n", mDate);
    str.AppendFormatted(4096, "RECALLID %i\n", mRecallId);

    mMasterTrack->persist(str);

    for (const auto track : mTracks) {
        track->persist(str);
    }

    for (const auto& info : mControlInfos) {
        ControlInfo_Persist(info.get(), str);
    }
}

bool LivePreset::initFromChunkHandler(std::string &key, std::vector<const char *> &params) {
    if (BaseInfo::initFromChunkHandler(key, params))
        return true;

    if (key == "GUID") {
        stringToGuid(params[0], &mGuid);
        return true;
    }
    if (key == "NAME") {
        mName = params[0];
        return true;
    }
    if (key == "DESC") {
        mDescription = params[0];
        return true;
    }
    if (key == "DATE") {
        mDate = std::stoi(params[0]);
        return true;
    }
    if (key == "RECALLID") {
        mRecallId = std::stoi(params[0]);
        return true;
    }
    return false;
}

bool LivePreset::initFromChunkHandler(std::string &key, ProjectStateContext *ctx) {
    if (key == "MASTERTRACKINFO") {
        mMasterTrack = new MasterTrackInfo(nullptr, ctx);
        return true;
    }
    if (key == "TRACKINFO") {
        mTracks.emplace_back(new TrackInfo(nullptr, ctx));
        return true;
    }
    if (key == "CONTROLINFO") {
        if (auto info = ControlInfo_Create(this, ctx)) {
            mControlInfos.emplace_back(info);
        }
        return true;
    }

    return BaseInfo::initFromChunkHandler(key, ctx);
}

void LivePreset::recallSettings() const {

    mMasterTrack->recallSettings();
    for (const auto track : mTracks) {
        track->recallSettings();
    }
    for (const auto& info : mControlInfos) {
        ControlInfo_RecallSettings(info.get());
    }
}

std::set<std::string> LivePreset::getKeys() const {
    return BaseInfo::getKeys();
}

char * LivePreset::getTreeText() const {
    std::string newText = Filterable::getFilterText() + " " + mName;
    newText.copy(mTreeText, newText.length());
    mTreeText[newText.length()] = '\0';
    return mTreeText;
}

std::string LivePreset::getChunkId() const {
    return "LIVEPRESET";
}

FilterPreset* LivePreset::extractFilterPreset() {
    FilterPreset::ItemIdentifier id{};
    id.guid = mGuid;

    auto childs = std::vector<FilterPreset*>();

    childs.push_back(mMasterTrack->extractFilterPreset());

    for (auto track : mTracks) {
        childs.push_back(track->extractFilterPreset());
    }

    for (const auto& ctrl : mControlInfos) {
        childs.push_back(ctrl->extractFilterPreset());
    }

    return new FilterPreset(id, LIVEPRESET, mFilter, childs);
}

bool LivePreset::applyFilterPreset(FilterPreset *preset) {
    if (preset->mType == LIVEPRESET) {
        mFilter = preset->mFilter;

        auto toFilters = std::set<Filterable*>();
        toFilters.insert((Filterable*) mMasterTrack);
        toFilters.insert(mTracks.begin(), mTracks.end());

        //convert shared_ptr to raw pointer
        auto temp = std::vector<ControlInfo*>();
        for (const auto& ctrl : mControlInfos) {
            temp.push_back(ctrl.get());
        }
        toFilters.insert(temp.begin(), temp.end());

        for (auto& child : preset->mChilds) {
            for (auto toFilter : toFilters) {
                if (toFilter->applyFilterPreset(child)) {
                    toFilters.erase(toFilter);
                    goto cnt;
                }
            }
            cnt:;
        }
        return true;
    }
    return false;
}