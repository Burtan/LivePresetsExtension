/******************************************************************************
/ LivePresetsExtension
/
/ Represents a saved state that can be recalled
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

#ifndef LPE_LIVEPRESET_H
#define LPE_LIVEPRESET_H

#include <ctime>
#include <memory>
#include <liblpe/data/models/TrackInfo.h>
#include <liblpe/data/models/MasterTrackInfo.h>
#include <liblpe/data/models/base/BaseCommand.h>

class ControlInfo;

class LivePreset : public BaseInfo {
public:
	explicit LivePreset(std::string name = "New preset", std::string description = "");
    explicit LivePreset(ProjectStateContext* ctx, BaseCommand::CommandID recallCmdId = 0);
    LivePreset& operator=(LivePreset&& other) noexcept;
    ~LivePreset();

    //transient data
    BaseCommand::CommandID mRecallCmdId = 0;
    std::string mRecallIdDisplayingString = "";

    //data to persist
    GUID mGuid = GUID();
    std::string mName;
    std::string mDescription;
    time_t mDate = time(nullptr);
    int mRecallId = -1;

    //data that can be recalled
    MasterTrackInfo* mMasterTrack = nullptr;
    std::vector<TrackInfo*> mTracks;
    std::vector<std::shared_ptr<ControlInfo>> mControlInfos;

    [[nodiscard]] char *getTreeText() const override;
    void recallSettings() const override;
    void saveCurrentState(bool update) override;
    FilterPreset* extractFilterPreset() override;
    bool applyFilterPreset(FilterPreset *preset) override;
    void createRecallAction();
protected:
    [[nodiscard]] std::set<std::string> getKeys() const override;
    void persistHandler(WDL_FastString &str) const override;
    bool initFromChunkHandler(std::string &key, std::vector<const char *> &params) override;
    bool initFromChunkHandler(std::string &key, ProjectStateContext *ctx) override;
private:
    [[nodiscard]] std::string getChunkId() const override;
};


#endif //LPE_LIVEPRESET_H