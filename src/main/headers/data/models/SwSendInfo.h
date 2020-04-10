/******************************************************************************
/ LivePresetsExtension
/
/ Represents a send object, respective receives are ignored as they are implicite with the sends
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

#ifndef LPE_SWSENDINFO_H
#define LPE_SWSENDINFO_H

#include <data/models/base/BaseSendInfo.h>

//define used parameters
#define P_DESTTRACK "P_DESTTRACK"

/**
 * Represents information about a send between two tracks (category 0)
 */
class SwSendInfo : public BaseSendInfo {
public:
    SwSendInfo(Filterable* parent, GUID srcGuid, int sendidx);
    explicit SwSendInfo(Filterable* parent, ProjectStateContext *ctx);

    GUID mDstTrackGuid = GUID();

    void saveCurrentState(bool update) override;
    [[nodiscard]] char *getTreeText() const override;

    void recallSettings() const override;
    FilterPreset* extractFilterPreset() override;
    bool applyFilterPreset(FilterPreset *preset) override;
protected:
    [[nodiscard]] std::set<std::string> getKeys() const override;
    void persistHandler(WDL_FastString &str) const override;
    bool initFromChunkHandler(std::string &key, std::vector<const char *> &params) override;
    [[nodiscard]] MediaTrack* getSrcTrack() const override;
    [[nodiscard]] MediaTrack* getDstTrack() const;
private:
    [[nodiscard]] std::string getChunkId() const override;
};

#endif //LPE_SWSENDINFO_H
