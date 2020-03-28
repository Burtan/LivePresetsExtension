//
// Created by frederik on 14/01/2020.
//

#include <plugins/reaper_plugin_functions.h>
#include <utility>
#include <util/util.h>
#include <data/models/FilterPreset.h>

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

    for (auto child : mChilds) {
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
    for (auto child : mChilds) {
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

std::vector<std::string*> FilterPreset_GetNames(const std::vector<FilterPreset*>& presets) {
    auto names = std::vector<std::string*>();
    for (auto preset : presets) {
        names.push_back(&preset->mId.name);
    }
    return names;
}

FilterPreset* FilterPreset_GetFilterByName(const std::vector<FilterPreset*>& presets, std::string* name) {
    for (auto preset : presets) {
        if (preset->mId.name == *name) {
            return preset;
        }
    }
    return nullptr;
}