/******************************************************************************
/ LivePresetsExtension
/
/ Same as TrackInfo but adapter to the master track thus not containing software sends and rec fx
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

#include <liblpe/data/models/MasterTrackInfo.h>
#include <reaper_plugin_functions.h>
#include <liblpe/data/models/FilterPreset.h>

MasterTrackInfo::MasterTrackInfo(Filterable* parent) : BaseTrackInfo(parent) {
    MasterTrackInfo::saveCurrentState(false);
}

MasterTrackInfo::MasterTrackInfo(Filterable* parent, ProjectStateContext* ctx) : BaseTrackInfo(parent) {
    initFromChunk(ctx);
}

void MasterTrackInfo::saveCurrentState(bool update) {
    BaseTrackInfo::saveCurrentState(update);

    //only manage sends and hardware outputs, receives are automatically created by sends
    saveHwSendState(update);
    BaseTrackInfo::saveFxState(nullptr, mFxs, getMediaTrack(), &MASTER_GUID, update, false);
}

void MasterTrackInfo::saveHwSendState(bool update) {
    if (update) {
        mHwSends.clear();
    }
    for (int i = 0; i < GetTrackNumSends(getMediaTrack(), 1); i++) {
        auto info = new HwSendInfo(this, MASTER_GUID, i);
        mHwSends.push_back(info);
    }
}


MediaTrack* MasterTrackInfo::getMediaTrack() const {
    return GetMasterTrack(nullptr);
}

std::string MasterTrackInfo::getChunkId() const {
    return "MASTERTRACKINFO";
}

std::set<std::string> MasterTrackInfo::getKeys() const {
    return BaseTrackInfo::getKeys();
}

char * MasterTrackInfo::getTreeText() const {
    std::string newText = getFilterText() + " Master";
    newText.copy(mTreeText, newText.length());
    mTreeText[newText.length()] = '\0';
    return mTreeText;
}

FilterPreset* MasterTrackInfo::extractFilterPreset() {
    FilterPreset::ItemIdentifier id{};

    auto childs = std::vector<FilterPreset*>();

    childs.push_back(mParamInfo.extractFilterPreset());

    for (auto hwSend : mHwSends) {
        childs.push_back(hwSend->extractFilterPreset());
    }
    for (auto fx : mFxs) {
        childs.push_back(fx->extractFilterPreset());
    }

    return new FilterPreset(id, MASTERTRACK, mFilter, childs);}

bool MasterTrackInfo::applyFilterPreset(FilterPreset *preset) {
    if (preset->mType == MASTERTRACK) {
        mFilter = preset->mFilter;
        auto toFilters = std::set<Filterable*>();
        toFilters.insert(&mParamInfo);
        toFilters.insert(mHwSends.begin(), mHwSends.end());
        toFilters.insert(mFxs.begin(), mFxs.end());

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

MasterTrackInfo::~MasterTrackInfo() {}
