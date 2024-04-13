/******************************************************************************
/ LivePresetsExtension
/
/ Bass class for TrackInfo and MasterTrackInfo
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

#ifndef LPE_BASETRACKINFO_H
#define LPE_BASETRACKINFO_H

#include <data/models/base/BaseInfo.h>
#include <data/models/FxInfo.h>
#include <data/models/SwSendInfo.h>
#include <data/models/HwSendInfo.h>

#define B_MUTE "B_MUTE"
#define I_SOLO "I_SOLO"
#define I_FXEN "I_FXEN"
#define I_AUTOMODE "I_AUTOMODE"
#define I_NCHAN "I_NCHAN"
#define I_SELECTED "I_SELECTED"
#define I_MIDIHWOUT "I_MIDIHWOUT"
#define I_PERFFLAGS "I_PERFFLAGS"
#define I_CUSTOMCOLOR "I_CUSTOMCOLOR"
#define I_HEIGHTOVERRIDE "I_HEIGHTOVERRIDE"
#define B_HEIGHTLOCK "B_HEIGHTLOCK"
#define D_VOL "D_VOL"
#define D_PAN "D_PAN"
#define D_WIDTH "D_WIDTH"
#define D_DUALPANL "D_DUALPANL"
#define D_DUALPANR "D_DUALPANR"
#define I_PANMODE "I_PANMODE"
#define D_PANLAW "D_PANLAW"
#define C_BEATATTACHMODE "C_BEATATTACHMODE"
#define F_MCP_FXSEND_SCALE "F_MCP_FXSEND_SCALE"
#define F_MCP_SENDRGN_SCALE "F_MCP_SENDRGN_SCALE"

class BaseTrackInfo : public BaseInfo {
public:
    static const GUID MASTER_GUID;

    BaseTrackInfo(Filterable* parent);
    ~BaseTrackInfo();

    //data to persist
    //data that can be recalled
    std::vector<FxInfo*> mFxs;
    std::vector<HwSendInfo*> mHwSends;

    void recallSettings() const override;
    void saveCurrentState(bool update) override;
protected:
    [[nodiscard]] std::set<std::string> getKeys() const override;
    bool initFromChunkHandler(std::string& key, std::vector<const char*>& params) override;
    bool initFromChunkHandler(std::string& key, ProjectStateContext *ctx) override;
    void persistHandler(WDL_FastString &str) const override;
    [[nodiscard]] virtual MediaTrack* getMediaTrack() const = 0;
    void recallHwSends() const;
    static void saveSwSendState(Filterable *parent, std::vector<SwSendInfo *> &swSends, MediaTrack *track,
            const GUID *guid, bool update);
    static void saveHwSendState(Filterable *parent, std::vector<HwSendInfo *> &hwSends, MediaTrack *track,
            const GUID *guid, bool update);
    static void saveFxState(Filterable *parent, std::vector<FxInfo *> &fxs, MediaTrack *track, const GUID *guid,
            bool update, bool rec = false);
};


#endif //LPE_BASETRACKINFO_H