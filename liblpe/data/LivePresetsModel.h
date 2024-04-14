/******************************************************************************
/ LivePresetsExtension
/
/ Highest order class of the data model
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

#ifndef LPE_LIVEPRESETSMODEL_H
#define LPE_LIVEPRESETSMODEL_H


#include <liblpe/data/models/LivePreset.h>
#include <liblpe/data/models/base/Persistable.h>
#include <liblpe/data/models/FilterPreset.h>
#include <liblpe/plugins/lpe_ultimate.h>

class LivePresetsModel : public Persistable {
public:
    static const int VERSION = 1;

    LivePresetsModel() = default;
    explicit LivePresetsModel(ProjectStateContext* ctx);
    LivePresetsModel& operator=(LivePresetsModel&& other) noexcept;
    ~LivePresetsModel();

    bool mDoUndo = false;
    bool mIsHideMutedTracks = false;
    bool mIsLoadStateOnMute = false;
    bool mIsReselectFxPreset = false;
    bool mIsReselectLivePresetByValueRecall = false;
    std::string mDefaultFilterPreset;
    std::vector<Hardware*> mHardwares;
    std::vector<LivePreset*> mPresets;
    std::vector<FilterPreset*> mFilterPresets;

    const LivePreset* getActivePreset();
    void recallByValue(int cc);
    int getRecallIdForPreset(LivePreset* preset, int id = 0);

    //undo redo relevant
    void replacePreset(LivePreset* oldPreset, LivePreset* newPreset);
    void removePreset(LivePreset* preset, bool saveUndo = true);
    void addPreset(LivePreset* preset, bool saveUndo = true);
    [[nodiscard]] LivePreset* getCurrentSettingsAsPreset() const;
    void removePresets(std::vector<LivePreset*> &preset);
    void recallPreset(LivePreset* preset);
    bool recallPresetByGuid(GUID guid);
    void onApplySelectedTrackConfigsToAllPresets(const std::vector<MediaTrack*>& tracks);
    void reset();
protected:
    void persistHandler(WDL_FastString &str) const override;
    bool initFromChunkHandler(std::string &key, std::vector<const char *> &params) override;
    bool initFromChunkHandler(std::string &key, ProjectStateContext *ctx) override;
private:
    LivePreset* mActivePreset = nullptr;
    [[nodiscard]] std::string getChunkId() const override;
};


#endif //LPE_LIVEPRESETSMODEL_H