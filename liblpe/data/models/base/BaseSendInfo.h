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

#ifndef LPE_SENDINFO_H
#define LPE_SENDINFO_H

#include <liblpe/data/models/base/BaseInfo.h>

//define used parameters
#define B_MUTE "B_MUTE"
#define B_MONO "B_MONO"
#define B_PHASE "B_PHASE"
#define D_VOL "D_VOL"
#define D_PAN "D_PAN"
#define D_PANLAW "D_PANLAW"
#define I_SENDMODE "I_SENDMODE"
#define I_AUTOMODE "I_AUTOMODE"
/**
 * -1 = none, 0 - 1023 = starting channel number,
 * range 0 - 1023 = stereo,
 * 1024 - 2047 mono channels,
 * 2048 - 3071 4 channels,
 * 3072 - 4095 6 channels,
 * 4096 - 5119 8 channels
 * 5120 - 6143 10 channels
 * to be continued
 */
#define I_SRCCHAN "I_SRCCHAN"
#define I_DSTCHAN "I_DSTCHAN"
#define I_MIDIFLAGS "I_MIDIFLAGS"

class BaseSendInfo : public BaseInfo {
public:
    explicit BaseSendInfo(Filterable* parent);
    BaseSendInfo(Filterable* parent, GUID trackGuid, int sendidx);

    //not persisted!!
    mutable std::string mName = "Send";
    mutable int mSendIdx = -1;

    //persisted
    GUID mSrcTrackGuid = GUID();

    void recallSettings() const override;
    void saveCurrentState(bool update) override;
protected:
    bool initFromChunkHandler(std::string &key, std::vector<const char*> &params) override;
    void persistHandler(WDL_FastString &str) const override;
    [[nodiscard]] std::set<std::string> getKeys() const override;
    [[nodiscard]] virtual MediaTrack* getSrcTrack() const = 0;
};


#endif //LPE_SENDINFO_H