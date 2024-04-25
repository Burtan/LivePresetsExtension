//
// Created by frederik on 01.02.20.
//

#include <reaper_plugin_functions.h>
#include <liblpe/data/models/Control.h>

Control::Control() {
    genGuid(&mGuid);
}

Control::Control(ProjectStateContext *ctx) {
    initFromChunk(ctx);
}

void Control::persistHandler(WDL_FastString &str) const {

}

bool Control::initFromChunkHandler(std::string &key, std::vector<const char *> &params) {
    return false;
}

bool Control::initFromChunkHandler(std::string &key, ProjectStateContext *ctx) {
    return false;
}

std::string Control::getChunkId() const {
    return "CONTROL";
}

const char *Control::getName(int index) const {
    snprintf(mNameText, 256, "%i: \n", index);
    return (const char*) mNameText;
}