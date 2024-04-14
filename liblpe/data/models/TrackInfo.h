/******************************************************************************
/ LivePresetsExtension
/
/ Represents a MediaTrack object with data
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

#ifndef LPE_TRACKINFO_H
#define LPE_TRACKINFO_H

#include <liblpe/data/models/base/BaseTrackInfo.h>

//define used parameters
#define B_PHASE "B_PHASE"
#define I_RECARM "I_RECARM"
#define I_RECINPUT "I_RECINPUT"
#define I_RECMODE "I_RECMODE"
#define I_RECMON "I_RECMON"
#define I_RECMONITEMS "I_RECMONITEMS"
#define I_FOLDERDEPTH "I_FOLDERDEPTH"
#define I_FOLDERCOMPACT "I_FOLDERCOMPACT"
#define B_SHOWINMIXER "B_SHOWINMIXER"
#define B_SHOWINTCP "B_SHOWINTCP"
#define B_MAINSEND "B_MAINSEND"
#define C_MAINSEND_OFFS "C_MAINSEND_OFFS"
#define B_FREEMODE "B_FREEMODE"

class TrackInfo : public BaseTrackInfo {
public:
    explicit TrackInfo(Filterable* parent, MediaTrack* track);
    explicit TrackInfo(Filterable* parent, ProjectStateContext* ctx);
    ~TrackInfo();

    //data to persist
    GUID mGuid = GUID();
    //data that can be recalled
    Parameter<std::string> mName = Parameter<std::string>(this, "NAME", "");
    std::vector<SwSendInfo*> mSwSends;
    std::vector<FxInfo*> mRecFxs;

    [[nodiscard]] char *getTreeText() const override;
    void saveCurrentState(bool update) override;

    void recallSettings() const override;
    FilterPreset* extractFilterPreset() override;
    bool applyFilterPreset(FilterPreset *preset) override;
protected:
    [[nodiscard]] std::set<std::string> getKeys() const override;
    bool initFromChunkHandler(std::string &key, std::vector<const char*> &params) override;
    bool initFromChunkHandler(std::string &key, ProjectStateContext *ctx) override;
    void persistHandler(WDL_FastString &str) const override;
    [[nodiscard]] MediaTrack *getMediaTrack() const override;
    [[nodiscard]] std::string getChunkId() const override;
};


#endif //LPE_TRACKINFO_H