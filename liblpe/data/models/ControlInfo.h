//
// Created by frederik on 01.02.20.
//

#ifndef LPE_CONTROLINFO_H
#define LPE_CONTROLINFO_H

#include <liblpe/data/models/base/BaseInfo.h>

class ControlInfo final : public BaseInfo {
public:
    explicit ControlInfo(Filterable* parent, GUID hwGuid, GUID ctrlGuid, GUID trackGuid, GUID fxGuid, int paramIndex);
    explicit ControlInfo(Filterable* parent, ProjectStateContext* ctx);

    GUID mHwGuid = GUID();
    GUID mCtrlGuid = GUID();
    GUID mTrackGuid = GUID();
    GUID mFxGuid = GUID();
    int mParamIndex = -1;

    void recallSettings() const override;
    void saveCurrentState(bool update) override;
    char *getTreeText() const override;
    FilterPreset* extractFilterPreset() override;
    bool applyFilterPreset(FilterPreset *preset) override;
protected:
    void persistHandler(WDL_FastString &str) const override;
    bool initFromChunkHandler(std::string &key, std::vector<const char *> &params) override;
    bool initFromChunkHandler(std::string &key, ProjectStateContext *ctx) override;
private:
    [[nodiscard]] std::string getChunkId() const override;
};

inline void ControlInfo_Persist(ControlInfo* info, WDL_FastString& str) {
    info->persist(str);
}
inline ControlInfo* ControlInfo_Create(Filterable* parent, ProjectStateContext* ctx) {
    return new ControlInfo(parent, ctx);
}
inline void ControlInfo_RecallSettings(ControlInfo* info) {
    info->recallSettings();
}

#endif //LPE_CONTROLINFO_H