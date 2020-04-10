/******************************************************************************
/ LivePresetsExtension
/
/ Represents a TrackFX object
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

#include <data/models/FxInfo.h>
#include <data/models/FilterPreset.h>
#include <util/util.h>
#include <cfloat>
#include <plugins/reaper_plugin_functions.h>
#include <LivePresetsExtension.h>

FxInfo::FxInfo(Filterable* parent, GUID trackGuid, GUID fxGuid) : BaseInfo(parent),
        mTrackGuid(trackGuid), mGuid(fxGuid) {
    FxInfo::saveCurrentState(false);
}

FxInfo::FxInfo(Filterable* parent, ProjectStateContext* ctx) : BaseInfo(parent) {
    initFromChunk(ctx);
}

void FxInfo::saveCurrentState(bool update) {
    int index = getCurrentIndex();
    mIndex = Parameter<int>(this, "INDEX", index, update ? mIndex.mFilter : RECALLED);

    //dont save any more info is the Fx cannot be found
    if (index == -1)
        return;

    char buffer[256] = "";
    TrackFX_GetFXName(getTrack(), index, buffer, sizeof(buffer));
    mName = buffer;

    TrackFX_GetPreset(getTrack(), index, buffer, sizeof(buffer));
    mPresetName = Parameter<std::string>(this, "PRESETNAME", buffer, update ? mPresetName.mFilter : RECALLED);

    mEnabled = Parameter<bool>(this, "ENABLED", TrackFX_GetEnabled(getTrack(), index), update ? mEnabled.mFilter : RECALLED);

    auto min = DBL_MIN;
    auto max = DBL_MAX;

    for (int i = 0; i < TrackFX_GetNumParams(getTrack(), index); i++) {
        auto filter = update ? (mParamInfo.keyExists(i) ? mParamInfo.at(i).mFilter : RECALLED) : RECALLED;
        auto param = Parameter<double>(&mParamInfo, i, TrackFX_GetParam(getTrack(), index, i, &min, &max), filter);
        mParamInfo.insert(i, param);
    }
}

void FxInfo::recallSettings() const {
    //dont continue recalling when parent filter or own filter is IGNORED
    if (isFilteredInChain())
        return;

    int index = getCurrentIndex();
    //dont recall any more info is the Fx cannot be found
    if (index == -1)
        return;

    if (!mIndex.isFilteredInChain() && index != mIndex.mValue) {
        TrackFX_CopyToTrack(getTrack(), getCurrentIndex(), getTrack(), mIndex.mValue, true);
        index = mIndex.mValue;
    }

    if (!mEnabled.isFilteredInChain() && TrackFX_GetEnabled(getTrack(), index) != mEnabled.mValue)
        TrackFX_SetEnabled(getTrack(), index, mEnabled.mValue);

    auto min = DBL_MIN;
    auto max = DBL_MAX;

    //FX Preset loading
    //has to be done every time as changes by the user on plugin presets is not tracked
    char name[256];
    TrackFX_GetFXName(getTrack(), index, (char*) name, 256);

    switch (g_lpe->mPrs.get(name)) {
        case PluginRecallStrategies::NONE:
            //don't do anything
            break;
        case PluginRecallStrategies::PRESET: {
            //load reaper preset
            TrackFX_GetPreset(getTrack(), index, (char*) name, 256);
            if (!mPresetName.isFilteredInChain() && (g_lpe->mModel.mIsReselectFxPreset || name != mPresetName.mValue.data())) {
                TrackFX_SetPreset(getTrack(), index, mPresetName.mValue.data());
            }
            break;
        }
        case PluginRecallStrategies::PARAMETERS: {
            //recall parameters once
            for (int i = 0; i < mParamInfo.size(); i++) {
                auto currentValue = TrackFX_GetParam(getTrack(), index, i, &min, &max);
                if (!mParamInfo.at(i).isFilteredInChain() && currentValue != mParamInfo.at(i).mValue)
                    TrackFX_SetParam(getTrack(), index, i, mParamInfo.at(i).mValue);
            }
            break;
        }
        case PluginRecallStrategies::PARAMETERS_RETRY:
            //recall parameters and check if it succeeded, try up to 5 times
            for (int i = 0; i < mParamInfo.size(); i++) {
                if (!mParamInfo.at(i).isFilteredInChain()) {
                    int count = 0;
                    auto currentValue = TrackFX_GetParam(getTrack(), index, i, &min, &max);

                    while (mParamInfo.at(i).mValue != currentValue && count < 5) {
                        TrackFX_SetParam(getTrack(), index, i, mParamInfo.at(i).mValue);
                        currentValue = TrackFX_GetParam(getTrack(), index, i, &min, &max);
                        count++;
                    }
                }
            }
            break;
    }

}

MediaTrack* FxInfo::getTrack() const {
    if (GuidsEqual(mTrackGuid, BaseTrackInfo::MASTER_GUID))
        return GetMasterTrack(nullptr);

    for (int i = 0; i < CountTracks(nullptr); ++i) {
        MediaTrack* track = GetTrack(nullptr, i);
        if (GuidsEqual(mTrackGuid, *GetTrackGUID(track)))
            return track;
    }
    return nullptr;
}

/*
 * Returns the current index of the fx guid. -1 when the fx was not found.
 */
int FxInfo::getCurrentIndex() const {
    //first check for Fx
    for (int i = 0; i < TrackFX_GetCount(getTrack()); i++) {
        if (GuidsEqual(*TrackFX_GetFXGUID(getTrack(), i), mGuid)) {
            return i;
        }
    }
    //then check for RecFx
    for (int i = RECFX_INDEX_FACTOR; i < RECFX_INDEX_FACTOR + TrackFX_GetRecCount(getTrack()); i++) {
        if (GuidsEqual(*TrackFX_GetFXGUID(getTrack(), i), mGuid)) {
            return i;
        }
    }
    return -1;
}

void FxInfo::persistHandler(WDL_FastString &str) const {
    BaseInfo::persistHandler(str);

    char dest[64];
    guidToString(&mGuid, dest);
    str.AppendFormatted(4096, "GUID %s\n", dest);

    guidToString(&mTrackGuid, dest);
    str.AppendFormatted(4096, "TRACKGUID %s\n", dest);

    str.AppendFormatted(4096, "NAME \"%s\"\n", mName.data());
    str.AppendFormatted(4096, "ENABLED %i %i\n", mEnabled.mValue, mEnabled.mFilter);
    str.AppendFormatted(4096, "INDEX %i %i\n", mIndex.mValue, mEnabled.mFilter);
    str.AppendFormatted(4096, "PRESET \"%s\" %i\n", mPresetName.mValue.data(), mPresetName.mFilter);
}

bool FxInfo::initFromChunkHandler(std::string &key, std::vector<const char*> &params) {
    if (key == "TRACKGUID") {
        stringToGuid(params[0], &mTrackGuid);
        return true;
    }
    if (key == "GUID") {
        stringToGuid(params[0], &mGuid);
        return true;
    }
    if (key == "NAME") {
        mName = params[0];
        return true;
    }
    if (key == "ENABLED") {
        mEnabled = Parameter<bool>(this, "ENABLED", std::stoi(params[0]), (FilterMode) std::stoi(params[1]));
        return true;
    }
    if (key == "INDEX") {
        mIndex = Parameter<int>(this, "INDEX", std::stoi(params[0]), (FilterMode) std::stoi(params[1]));
        return true;
    }
    if (key == "PRESET") {
        mPresetName = Parameter<std::string>(this, "PRESET", params[0], (FilterMode) std::stoi(params[1]));
        return true;
    }

    return BaseInfo::initFromChunkHandler(key, params);
}

std::set<std::string> FxInfo::getKeys() const {
    auto set = BaseInfo::getKeys();
    for (int i = 0; i < TrackFX_GetNumParams(getTrack(), getCurrentIndex()); i++) {
        set.insert(std::to_string(i));
    }
    return set;
}

char * FxInfo::getTreeText() const {
    std::string newText = getFilterText() + " " + mName;
    newText.copy(mTreeText, newText.length());
    mTreeText[newText.length()] = '\0';
    return mTreeText;
}

std::string FxInfo::getChunkId() const {
    return "FXINFO";
}

FilterPreset* FxInfo::extractFilterPreset() {
    FilterPreset::ItemIdentifier id{};
    id.guid = mGuid;

    auto childs = std::vector<FilterPreset*>();

    childs.push_back(mParamInfo.extractFilterPreset());
    childs.push_back(mEnabled.extractFilterPreset());
    childs.push_back(mIndex.extractFilterPreset());
    childs.push_back(mPresetName.extractFilterPreset());

    return new FilterPreset(id, FX, mFilter, childs);
}

bool FxInfo::applyFilterPreset(FilterPreset *preset) {
    if (preset->mType == FX && GuidsEqual(preset->mId.guid, mGuid)) {
        mFilter = preset->mFilter;

        auto toFilters = std::set<Filterable*>();
        toFilters.insert((Filterable*) &mParamInfo);
        toFilters.insert((Filterable*) &mEnabled);
        toFilters.insert((Filterable*) &mIndex);
        toFilters.insert((Filterable*) &mPresetName);

        for (auto child : preset->mChilds) {
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
    return false;}