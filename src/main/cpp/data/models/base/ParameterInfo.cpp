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

#include <utility>
#include <set>
#include <data/models/base/ParameterInfo.h>
#include <data/models/FilterPreset.h>


ParameterInfo::ParameterInfo(ProjectStateContext* ctx) {
    initFromChunk(ctx);
}

void ParameterInfo::insert(const std::string& key, const Parameter<double>& value) {
    mParams.erase(key);
    mParams.insert(std::make_pair(key, value));
}

void ParameterInfo::insert(int key, const Parameter<double> &value) {
    insert(std::to_string(key), value);
}

const Parameter<double>& ParameterInfo::at(const std::string& key) const {
    return mParams.at(key);
}

const Parameter<double>& ParameterInfo::at(int key) const {
    return at(std::to_string(key));
}

int ParameterInfo::size() const {
    return mParams.size();
}

void ParameterInfo::clear() {
    mParams.clear();
}

void ParameterInfo::persistHandler(WDL_FastString &str) const {
    for (const auto& pair : mParams) {
        const auto& param = pair.second;
        str.AppendFormatted(4096, (pair.first + " %f %i\n").data(), param.mValue, param.mFilter);
    }

    str.AppendFormatted(4096, "FILTERMODE %u\n", mFilter);
}

bool ParameterInfo::initFromChunkHandler(std::string &key, std::vector<const char*> &params) {
    if (key == "FILTERMODE") {
        mFilter = (FilterMode) std::stoi(params[0]);
    } else {
        auto value = Parameter<double>(key, std::stod(params[0]), (FilterMode) std::stoi(params[1]));
        mParams.insert(std::make_pair(key, value));
    }
    return true;
}

char* ParameterInfo::getTreeText() const {
    std::string newText = getFilterText() + " Parameters";
    newText.copy(mTreeText, newText.length());
    mTreeText[newText.length()] = '\0';
    return mTreeText;
}

std::string ParameterInfo::getChunkId() const {
    return "PARAMETERINFO";
}

std::vector<std::string> ParameterInfo::getKeys() {
    std::vector<std::string> retval;
    for (auto const& element : mParams) {
        retval.push_back(element.first);
    }
    return retval;
}

bool ParameterInfo::keyExists(int key) const {
    auto it = mParams.find(std::to_string(key));
    return it != mParams.end();
}

FilterPreset* ParameterInfo::extractFilterPreset() {
    FilterPreset::ItemIdentifier id{};
    auto childs = std::vector<FilterPreset*>();
    for (auto param : mParams) {
        childs.push_back(param.second.extractFilterPreset());
    }
    return new FilterPreset(id, PARAMS, mFilter, childs);
}

bool ParameterInfo::applyFilterPreset(FilterPreset *preset) {
    if (preset->mType == PARAMS) {
        mFilter = preset->mFilter;

        auto toFilters = std::set<Filterable*>();
        for (const auto& param : mParams) {
            toFilters.insert((Filterable*) &param.second);
        }

        for (auto child : preset->mChilds) {
            for (auto toFilter : toFilters) {
                if (toFilter->applyFilterPreset(child)) {
                    toFilters.erase(toFilter);
                    goto cnt;
                }
            }
            cnt:;
        }
        return true;
    }
    return false;
}