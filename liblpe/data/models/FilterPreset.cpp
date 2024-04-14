/******************************************************************************
/ LivePresetsExtension
/
/ Represents a filter preset
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

#include <liblpe/plugins/reaper_plugin_functions.h>
#include <utility>
#include <liblpe/util/util.h>
#include <liblpe/data/models/FilterPreset.h>
#include <algorithm>

FilterPreset::FilterPreset(ItemIdentifier id, TYPE type, FilterMode filter, std::vector<FilterPreset*> childs)
        : mId(std::move(id)), mType(type), mFilter(filter), mChilds(std::move(childs)) {}

FilterPreset::FilterPreset(ProjectStateContext *ctx) {
    initFromChunk(ctx);
}

void FilterPreset::persistHandler(WDL_FastString &str) const {
    char dest[64];
    guidToString(&mId.guid, dest);
    str.AppendFormatted(4096, "ID \"%s\" \"%s\" %i \"%s\"\n", dest, mId.key.data(), mId.data, mId.name.data());

    str.AppendFormatted(4096, "FILTER %i\n", mFilter);
    str.AppendFormatted(4096, "TYPE %i\n", mType);

    for (auto *child : mChilds) {
        child->persist(str);
    }
}

bool FilterPreset::initFromChunkHandler(std::string &key, std::vector<const char*> &params) {
    if (key == "ID") {
        stringToGuid(params[0], &mId.guid);
        mId.key = params[1];
        mId.data = std::stoi(params[2]);
        mId.name = params[3];
        return true;
    }
    if (key == "FILTER") {
        mFilter = (FilterMode) std::stoi(params[0]);
        return true;
    }
    if (key == "TYPE") {
        mType = (TYPE) std::stoi(params[0]);
        return true;
    }
    return false;
}

bool FilterPreset::initFromChunkHandler(std::string &key, ProjectStateContext *ctx) {
    if (key == "FILTERPRESET") {
        mChilds.push_back(new FilterPreset(ctx));
        return true;
    }
    return false;
}

std::string FilterPreset::getChunkId() const {
    return "FILTERPRESET";
}

FilterPreset::~FilterPreset() {
    for (auto *child : mChilds) {
        delete child;
    }
    mChilds.clear();
}

void FilterPreset_AddPreset(std::vector<FilterPreset*>& presets, FilterPreset* newPreset) {
    for (int i = (int) presets.size() - 1; i >= 0; i--) {
        FilterPreset* preset = presets.at(i);
        if (preset->mId.name == newPreset->mId.name) {
            presets.erase(presets.begin() + i);
            delete preset;
        }
    }
    presets.push_back(newPreset);
}

void FilterPreset_SortChilds(FilterPreset *a) {
    bool (*compare)(FilterPreset*, FilterPreset*) = [](FilterPreset* a, FilterPreset* b) -> bool {
        if (a->mId.data != b->mId.data) {
            return a->mId.data >= b->mId.data;
        }
        if (!GuidsEqual(a->mId.guid, b->mId.guid)) {
            char ag[256];
            char bg[256];
            guidToString(&a->mId.guid, ag);
            guidToString(&b->mId.guid, bg);
            std::string as = ag;
            std::string bs = bg;
            return as >= bs;
        }
        if (a->mId.key != b->mId.key) {
            return a->mId.key >= b->mId.key;
        }
        if (a->mId.name != b->mId.name) {
            return a->mId.name >= b->mId.name;
        }

        return true;
    };

    std::sort(a->mChilds.begin(), a->mChilds.end(), compare);
}

bool FilterPreset_IsEqual(FilterPreset *a, FilterPreset *b) {
    //check if FilterPreset is equal
    if (a->mType == b->mType &&
            a->mFilter == b->mFilter &&
            a->mId.data == b->mId.data &&
            a->mId.key == b->mId.key &&
            GuidsEqual(a->mId.guid, b->mId.guid)
        ) {
        if (a->mChilds.empty() && b->mChilds.empty()) {
            return true;
        }
        //check if childs are equal
        FilterPreset_SortChilds(a);
        FilterPreset_SortChilds(b);
        for (auto *ac : a->mChilds) {
            for (auto *bc : b->mChilds) {
                if (FilterPreset_IsEqual(ac, bc)) {
                    goto next;
                }
            }
            return false;
            next:;
        }
        return true;
    }
    return false;
}

std::vector<std::string*> FilterPreset_GetNames(const std::vector<FilterPreset*>& presets) {
    auto names = std::vector<std::string*>();
    for (auto *preset : presets) {
        names.push_back(&preset->mId.name);
    }
    return names;
}

FilterPreset* FilterPreset_GetFilterByName(const std::vector<FilterPreset*>& presets, std::string* name) {
    for (auto *preset : presets) {
        if (preset->mId.name == *name) {
            return preset;
        }
    }
    return nullptr;
}