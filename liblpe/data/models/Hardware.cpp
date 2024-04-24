//
// Created by frederik on 02.02.20.
//

#include <liblpe/data/models/FilterPreset.h>
#include <liblpe/data/models/Hardware.h>
#include <reaper_plugin_functions.h>
#include <liblpe/util/util.h>

Hardware::Hardware() {
    genGuid(&mGuid);
}

Hardware::Hardware(ProjectStateContext *ctx) {
    initFromChunk(ctx);
}

Hardware::~Hardware() {
    for (auto control : mControls) {
        delete control;
    }
    mControls.clear();
    mControlInfos.clear();
}

void Hardware::persistHandler(WDL_FastString &str) const {
    str.AppendFormatted(4096, "NAME \"%s\"\n", mName.data());

    char dest[64];
    guidToString(&mGuid, dest);
    str.AppendFormatted(4096, "GUID %s\n", dest);

    for (auto& control : mControls) {
        control->persist(str);
    }
}

bool Hardware::initFromChunkHandler(std::string &key, std::vector<const char *> &params) {
    if (key == "GUID") {
        stringToGuid(params[0], &mGuid);
        return true;
    }

    if (key == "NAME") {
        mName = params[0];
        return true;
    }

    return false;
}

bool Hardware::initFromChunkHandler(std::string &key, ProjectStateContext *ctx) {
    if (key == "CONTROL") {
        mControls.emplace_back(new Control(ctx));
        return true;
    }
    return false;
}

std::string Hardware::getChunkId() const {
    return "HARDWARE";
}

std::shared_ptr<ControlInfo> Hardware::getControlInfoForControl(Control *control) const {
    for (auto info : mControlInfos) {
        if (GuidsEqual(control->mGuid, info->mCtrlGuid)) {
            return info;
        }
    }
    return nullptr;
}

void Hardware::addControlInfo(const std::shared_ptr<ControlInfo>& info) {
    mControlInfos.push_back(info);
}
