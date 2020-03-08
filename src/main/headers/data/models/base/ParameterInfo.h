/******************************************************************************
/ LivePresetsExtension
/
/ A class that holds parameters that can be filtered and persisted
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

#ifndef LPE_PARAMLIST_H
#define LPE_PARAMLIST_H


#include <data/models/base/Parameter.h>
#include <data/models/base/Persistable.h>
#include <map>

class ParameterInfo : public Filterable, public Persistable {
public:
    ParameterInfo() = default;
    explicit ParameterInfo(ProjectStateContext* ctx);

    [[nodiscard]] std::vector<std::string> getKeys();
    [[nodiscard]] const Parameter<double>& at(const std::string &key) const;
    [[nodiscard]] const Parameter<double>& at(int key) const;
    void insert(const std::string &key, const Parameter<double> &value);
    void insert(int key, const Parameter<double> &value);
    [[nodiscard]] int size() const;
    void clear();
    [[nodiscard]] bool keyExists(int key) const;
    [[nodiscard]] char* getTreeText() const override;
    FilterPreset* extractFilterPreset() override;
    bool applyFilterPreset(FilterPreset *preset) override;

    std::map<std::string, Parameter<double>> mParams;
protected:
    void persistHandler(WDL_FastString &str) const override;
    bool initFromChunkHandler(std::string &key, std::vector<const char*> &params) override;
    bool initFromChunkHandler(std::string &key, ProjectStateContext *ctx) override { return false; };
private:
    [[nodiscard]] std::string getChunkId() const override;
};


#endif //LPE_PARAMLIST_H