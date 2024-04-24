//
// Created by frederik on 01.02.20.
//

#ifndef LPE_CONTROL_H
#define LPE_CONTROL_H


#include <memory>
#include <liblpe/data/models/base/Persistable.h>
#include <liblpe/data/models/ActionCommand.h>

class Control final : public Persistable {
public:
    Control();
    explicit Control(ProjectStateContext *ctx);

    GUID mGuid = GUID();

    std::unique_ptr<ActionCommand> command;
    const char *getName(int index) const;
protected:
    void persistHandler(WDL_FastString &str) const override;
    bool initFromChunkHandler(std::string &key, std::vector<const char *> &params) override;
    bool initFromChunkHandler(std::string &key, ProjectStateContext *ctx) override;
private:
    mutable char mNameText[256] = "";
    [[nodiscard]] std::string getChunkId() const override;
};


#endif //LPE_CONTROL_H