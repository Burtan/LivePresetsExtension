/******************************************************************************
/ LivePresetsExtension
/
/ Represents a hardware send
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

#ifndef LPE_HWSENDINFO_H
#define LPE_HWSENDINFO_H

#include <liblpe/data/models/base/BaseSendInfo.h>

/**
 * Represents information about a send from tracks to hardware (category 1)
 */
class HwSendInfo : public BaseSendInfo {
public:
    HwSendInfo(Filterable* parent, GUID srcGuid, int sendidx);
    explicit HwSendInfo(Filterable* parent, ProjectStateContext *ctx);

    [[nodiscard]] char *getTreeText() const override;

    void saveCurrentState(bool update) override;
    void recallSettings() const override;
    FilterPreset* extractFilterPreset() override;
    bool applyFilterPreset(FilterPreset *preset) override;
protected:
    [[nodiscard]] std::set<std::string> getKeys() const override;
    void persistHandler(WDL_FastString &str) const override;
    bool initFromChunkHandler(std::string &key, std::vector<const char *> &params) override;
    [[nodiscard]] MediaTrack *getSrcTrack() const override;
private:
    [[nodiscard]] std::string getChunkId() const override;
};


#endif //LPE_HWSENDINFO_H