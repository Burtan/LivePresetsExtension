//
// Created by Frederik on 01.02.2020.
//

#include <liblpe/data/models/ControlInfo.h>
#include <liblpe/data/models/FilterPreset.h>
#include <reaper_plugin_functions.h>
#include <liblpe/util/util.h>

ControlInfo::ControlInfo(Filterable* parent, GUID hwGuid, GUID ctrlGuid, GUID trackGuid, GUID fxGuid, int paramIndex)
        : BaseInfo(parent), mHwGuid(hwGuid), mCtrlGuid(ctrlGuid), mTrackGuid(trackGuid), mFxGuid(fxGuid), mParamIndex(paramIndex) {
}

ControlInfo::ControlInfo(Filterable* parent, ProjectStateContext *ctx) : BaseInfo(parent) {
    initFromChunk(ctx);
}

void ControlInfo::recallSettings() const {

}

void ControlInfo::saveCurrentState(bool update) {

}

void ControlInfo::persistHandler(WDL_FastString &str) const {
    char dst[64];
    guidToString(&mTrackGuid, dst);
    str.AppendFormatted(4096, "TRACKGUID %s\n", dst);

    guidToString(&mFxGuid, dst);
    str.AppendFormatted(4096, "FXGUID %s\n", dst);

    guidToString(&mHwGuid, dst);
    str.AppendFormatted(4096, "HWGUID %s\n", dst);

    guidToString(&mCtrlGuid, dst);
    str.AppendFormatted(4096, "CTRLGUID %s\n", dst);

    str.AppendFormatted(4096, "INDEX %i\n", mParamIndex);
}

bool ControlInfo::initFromChunkHandler(std::string &key, std::vector<const char *> &params) {
    if (key == "TRACKGUID") {
        stringToGuid(params[0], &mTrackGuid);
        return true;
    }
    if (key == "FXGUID") {
        stringToGuid(params[0], &mFxGuid);
        return true;
    }
    if (key == "HWGUID") {
        stringToGuid(params[0], &mHwGuid);
        return true;
    }
    if (key == "CTRLGUID") {
        stringToGuid(params[0], &mCtrlGuid);
        return true;
    }
    if (key == "INDEX") {
        mParamIndex = std::stoi(params[0]);
        return true;
    }

    return false;
}

bool ControlInfo::initFromChunkHandler(std::string &key, ProjectStateContext *) {
    return false;
}

std::string ControlInfo::getChunkId() const {
    return "CONTROLINFO";
}

char *ControlInfo::getTreeText() const {
    //TODO improve tree text
    std::string newText = Filterable::getFilterText() + " ";
    newText.copy(mTreeText, newText.length());
    mTreeText[newText.length()] = '\0';
    return mTreeText;
}

FilterPreset* ControlInfo::extractFilterPreset() {
    FilterPreset::ItemIdentifier id{};
    id.guid = mCtrlGuid;

    return new FilterPreset(id, CTRL, mFilter);
}

bool ControlInfo::applyFilterPreset(FilterPreset *preset) {
    if (preset->mType == CTRL && GuidsEqual(preset->mId.guid, mCtrlGuid)) {
        mFilter = preset->mFilter;
        return true;
    }
    return false;
}