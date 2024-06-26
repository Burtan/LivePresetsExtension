/******************************************************************************
/ LivePresetsExtension
/
/ Base class for SwSendInfo and HwSendInfo
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

#include <liblpe/data/models/base/BaseSendInfo.h>
#include <reaper_plugin_functions.h>

BaseSendInfo::BaseSendInfo(Filterable *parent) : BaseInfo(parent) {}

/**
 * Create a new SendInfo object from a track
 * @param trackGuid the track guid
 * @param sendidx the id of the send
 */
BaseSendInfo::BaseSendInfo(Filterable* parent, GUID trackGuid, int sendidx) : BaseInfo(parent),
        mSendIdx(sendidx), mSrcTrackGuid(trackGuid) {
    BaseSendInfo::saveCurrentState(false);
}

void BaseSendInfo::saveCurrentState(bool update) {}

bool BaseSendInfo::initFromChunkHandler(std::string& key, std::vector<const char*>& params) {
    if (key == "SRCGUID") {
        stringToGuid(params[0], &mSrcTrackGuid);
        return true;
    }

    mParamInfo.insert(key, Parameter<double>(&mParamInfo, key, std::stod(params[0]), (FilterMode) std::stoi(params[1])));
    return true;
}

void BaseSendInfo::recallSettings() const {
    if (isFilteredInChain())
        return;
}

void BaseSendInfo::persistHandler(WDL_FastString &str) const {
    BaseInfo::persistHandler(str);

    char src[64];
    guidToString(&mSrcTrackGuid, src);

    str.AppendFormatted(4096, "SRCGUID %s\n", src);
}

std::set<std::string> BaseSendInfo::getKeys() const {
    auto set = BaseInfo::getKeys();
    std::set<std::string> keys = {
            B_MUTE,
            B_MONO,
            B_PHASE,
            D_VOL,
            D_PAN,
            D_PANLAW,
            I_SENDMODE,
            I_AUTOMODE,
            I_SRCCHAN,
            I_DSTCHAN,
            I_MIDIFLAGS
    };
    set.insert(keys.begin(), keys.end());
    return set;
}