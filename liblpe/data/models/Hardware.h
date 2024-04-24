//
// Created by frederik on 01.02.20.
//

#ifndef LPE_CONTROLPAGE_H
#define LPE_CONTROLPAGE_H

#ifdef _WIN32
    #include <guiddef.h>
#else
    #include <swell/swell-types.h>
#endif

#include <liblpe/data/models/base/Persistable.h>
#include <liblpe/data/models/Control.h>
#include <liblpe/data/models/ControlInfo.h>

class Hardware final : public Persistable {
public:
    Hardware();
    explicit Hardware(ProjectStateContext *ctx);
    ~Hardware();

    GUID mGuid = GUID();
    std::string mName = "New Hardware";
    std::vector<Control*> mControls;

    std::shared_ptr<ControlInfo> getControlInfoForControl(Control* control) const;
    void addControlInfo(const std::shared_ptr<ControlInfo>& info);
private:
    std::vector<std::shared_ptr<ControlInfo>> mControlInfos;

    void persistHandler(WDL_FastString &str) const override;
    bool initFromChunkHandler(std::string &key, std::vector<const char *> &params) override;
    bool initFromChunkHandler(std::string &key, ProjectStateContext *ctx) override;
    [[nodiscard]] std::string getChunkId() const override;
};

inline void Hardware_Persist(Hardware* hw, WDL_FastString& str) {
    hw->persist(str);
}
inline Hardware* Hardware_Create(ProjectStateContext* ctx) {
    return new Hardware(ctx);
}


#endif //LPE_CONTROLPAGE_H