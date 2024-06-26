/******************************************************************************
/ LivePresetsExtension
/
/ Highest order class of the data model
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

#include <liblpe/data/LivePresetsModel.h>
#include <algorithm>
#include <reaper_plugin_functions.h>
#include <liblpe/LivePresetsExtension.h>
#include <liblpe/util/util.h>

/*
 * Should be called to load LivePresetsModel from .rpp file.
 * ctx should contain the lines after <LIVEPRESETSMODEL
 */
LivePresetsModel::LivePresetsModel(ProjectStateContext *ctx) {
    initFromChunk(ctx);
}

LivePresetsModel::~LivePresetsModel() {
    reset();
}

/**
 * Move assignment for LivePresetsModel
 */
LivePresetsModel& LivePresetsModel::operator=(LivePresetsModel&& other) noexcept {
    //reassign all rvalues of rvalue reference other
    mHardwares = other.mHardwares;
    mPresets = other.mPresets;
    mFilterPresets = other.mFilterPresets;
    mActivePreset = other.mActivePreset;
    mDoUndo = other.mDoUndo;
    mIsHideMutedTracks = other.mIsHideMutedTracks;
    mIsLoadStateOnMute = other.mIsLoadStateOnMute;
    mIsReselectFxPreset = other.mIsReselectFxPreset;
    mIsReselectLivePresetByValueRecall = other.mIsReselectLivePresetByValueRecall;
    mDefaultFilterPreset = other.mDefaultFilterPreset;

    //make all pointers null and create empty containers for now empty instance other that its destruction does not
    //affect this instance
    other.mHardwares = std::vector<Hardware*>();
    other.mPresets = std::vector<LivePreset*>();
    other.mFilterPresets = std::vector<FilterPreset*>();
    other.mActivePreset = nullptr;

    return *this;
}

bool LivePresetsModel::initFromChunkHandler(std::string &key, std::vector<const char *> &params) {
    if (key == "VERSION") {
        //TODO check correct version
        return true;
    }
    if (key == "UNDO") {
        mDoUndo = (bool) std::stoi(params[0]);
        return true;
    }
    if (key == "HIDEMUTEDTRACKS") {
        mIsHideMutedTracks = (bool) std::stoi(params[0]);
        return true;
    }
    if (key == "RESELECTPRESETS") {
        mIsReselectLivePresetByValueRecall = (bool) std::stoi(params[0]);
        return true;
    }
    if (key == "RESELECTFXPRESETS") {
        mIsReselectFxPreset = (bool) std::stoi(params[0]);
        return true;
    }
    if (key == "LOADMUTED") {
        mIsLoadStateOnMute = (bool) std::stoi(params[0]);
        return true;
    }
    if (key == "DEFAULTFILTER") {
        mDefaultFilterPreset = params[0];
        return true;
    }
    return false;
}

bool LivePresetsModel::initFromChunkHandler(std::string &key, ProjectStateContext *ctx) {
    if (key == "LIVEPRESET") {
        mPresets.push_back(new LivePreset(ctx));
        return true;
    }
    if (key == "FILTERPRESET") {
        mFilterPresets.push_back(new FilterPreset(ctx));
        return true;
    }
    if (key == "HARDWARE") {
        if (auto* hw = Hardware_Create(ctx)) {
            mHardwares.push_back(hw);
        }
        return true;
    }
    return false;
}

LivePreset *LivePresetsModel::getCurrentSettingsAsPreset() const {
    return new LivePreset("new preset " + std::to_string(mPresets.size() + 1));
}

void LivePresetsModel::addPreset(LivePreset *preset, bool saveUndo) {
    mPresets.push_back(preset);
    if (saveUndo) {
        Undo_OnStateChangeEx2(nullptr, "Add LivePreset", UNDO_STATE_MISCCFG, -1);
    }
}

/*
 * Tries to find a preset with the given guid and recalls it
 * returns true when a preset was found
 */
bool LivePresetsModel::recallPresetByGuid(GUID guid) {
    for (auto* preset : mPresets) { // NOLINT(readability-use-anyofallof)
        if (GuidsEqual(preset->mGuid, guid)) {
            recallPreset(preset);
            return true;
        }
    }
    return false;
}


void LivePresetsModel::recallPreset(LivePreset* preset) {
    if (!preset)
        return;

    if (mDoUndo) {
        Undo_BeginBlock();
        PreventUIRefresh(1);
        preset->recallSettings();
        mActivePreset = preset;
        if (g_lpe->mController.mList)
            g_lpe->mController.mList->invalidate();
        PreventUIRefresh(-1);
        Undo_OnStateChangeEx2(nullptr, "Recall LivePreset", UNDO_STATE_ALL, -1);
        Undo_EndBlock("Recall LivePreset", UNDO_STATE_ALL);
    } else {
        PreventUIRefresh(1);
        preset->recallSettings();
        mActivePreset = preset;
        if (g_lpe->mController.mList)
            g_lpe->mController.mList->invalidate();
        TrackList_AdjustWindows(true);
        PreventUIRefresh(-1);
    }
}

void LivePresetsModel::replacePreset(LivePreset *oldPreset, LivePreset *newPreset) {
    mPresets.erase(remove(mPresets.begin(), mPresets.end(), oldPreset), mPresets.end());
    mPresets.push_back(newPreset);
}

/**
 * Removes a preset from model, also removes the assigned preset recall binding
 * @param preset
 * @param saveUndo
 */
void LivePresetsModel::removePreset(LivePreset* preset, bool saveUndo) {
    mPresets.erase(remove(mPresets.begin(), mPresets.end(), preset), mPresets.end());
    g_lpe->mActions.remove(preset->mRecallCmdId);
    if (saveUndo) {
        Undo_OnStateChangeEx2(nullptr, "Remove LivePreset", UNDO_STATE_MISCCFG, -1);
    }
}

void LivePresetsModel::removePresets(std::vector<LivePreset*>& presets) {
    for (auto* preset : presets) {
        removePreset(preset, false);
    }
    Undo_OnStateChangeEx2(nullptr, "Remove src", UNDO_STATE_MISCCFG, -1);
}

void LivePresetsModel::persistHandler(WDL_FastString &str) const {
    //add attributes
    str.AppendFormatted(4096, "VERSION %d\n", VERSION);
    str.AppendFormatted(4096, "UNDO %d\n", mDoUndo);
    str.AppendFormatted(4096, "HIDEMUTEDTRACKS %d\n", mIsHideMutedTracks);
    str.AppendFormatted(4096, "RESELECTPRESETS %d\n", mIsReselectLivePresetByValueRecall);
    str.AppendFormatted(4096, "RESELECTFXPRESETS %d\n", mIsReselectFxPreset);
    str.AppendFormatted(4096, "LOADMUTED %d\n", mIsLoadStateOnMute);
    str.AppendFormatted(4096, "DEFAULTFILTER \"%s\"\n", mDefaultFilterPreset.data());

    //add objects
    for (const auto* preset : mPresets) {
        preset->persist(str);
    }

    for (const auto* preset : mFilterPresets) {
        preset->persist(str);
    }

    for (auto* hardware : mHardwares) {
        Hardware_Persist(hardware, str);
    }
}

void LivePresetsModel::recallByValue(int val) {
    for (auto* preset : mPresets) {
        if (preset->mRecallId == val) {
            if (mIsReselectLivePresetByValueRecall || !mActivePreset || preset != mActivePreset)
                recallPreset(preset);
            break;
        }
    }
}

/**
 * Use this function to assign recallIds for presets instead of the preset itself to prevent non-unique IDs
 * @param preset the preset to assign the unique id to
 * @param id the unique id
 */
int LivePresetsModel::getRecallIdForPreset(LivePreset* preset, int id) {
    for (auto* mPreset : mPresets) {
        if ((mPreset->mRecallId == id && !GuidsEqual(preset->mGuid, mPreset->mGuid)) || id == -1) {
            return getRecallIdForPreset(preset, id + 1);
        }
    }
    return id;
}

/**
 * Saves the current track configs in all presets
 * @param tracks tracks to save
 */
void LivePresetsModel::onApplySelectedTrackConfigsToAllPresets(const std::vector<MediaTrack*>& tracks) {
    for (auto* preset : mPresets) {
        for (auto* updatingTrack : tracks) {
            auto guid = *GetTrackGUID(updatingTrack);

            TrackInfo* updatingTrackInfo = nullptr;
            for (auto* mTrackInfo : preset->mTracks) {
                if (GuidsEqual(guid, mTrackInfo->mGuid)) {
                    updatingTrackInfo = mTrackInfo;
                    break;
                }
            }

            if (updatingTrackInfo == nullptr) {
                // no existing TrackInfo on preset was found, create a new one
                updatingTrackInfo = new TrackInfo(nullptr, updatingTrack);
                preset->mTracks.push_back(updatingTrackInfo);
                updatingTrackInfo->saveCurrentState(false);
            } else {
                // update existing TrackInfo
                updatingTrackInfo->saveCurrentState(true);
            }

            preset->mDate = time(nullptr);
        }
    }
}

const LivePreset* LivePresetsModel::getActivePreset() {
    return mActivePreset;
}

std::string LivePresetsModel::getChunkId() const {
    return "LIVEPRESETSMODEL";
}

void LivePresetsModel::reset() {
    for (auto* hardware : mHardwares) {
        delete hardware;
    }
    mHardwares.clear();

    for (auto* preset : mPresets) {
        delete preset;
    }
    mPresets.clear();
}