/******************************************************************************
/ LivePresetsExtension
/
/ Bass class for recallable data including filter and persisting functions
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

#include <data/models/base/BaseInfo.h>

BaseInfo::BaseInfo(Filterable *parent) : Filterable(parent) {}

/**
 * Function to help persisting data
 * BaseInfo persists ParameterInfo and Filter
 * @param str add data to this string
 */
void BaseInfo::persistHandler(WDL_FastString &str) const {
    mParamInfo.persist(str);
    str.AppendFormatted(4096, "FILTERMODE %u\n", mFilter);
}

/**
 * Called when the object was constructed from a reaper chunk and contains additional elements
 * BASEINFO handles PARAMETERINFO
 * @param key element key
 * @param ctx reaper chunk data
 */
bool BaseInfo::initFromChunkHandler(std::string &key, ProjectStateContext* ctx) {
    if (key == "PARAMETERINFO") {
        mParamInfo = ParameterInfo(this, ctx);
        //fix parent after copy assignment
        for (auto& entry : mParamInfo.mParams) {
            entry.second.mParent = &mParamInfo;
        }
        return true;
    }
    return false;
}

/**
 * Called when the object was constructed from a reaper chunk and contains key/value data
 * BaseInfo handles FILTERMODE
 * @param key data key
 * @param params data values
 */
bool BaseInfo::initFromChunkHandler(std::string &key, std::vector<const char*> &params) {
    if (key == "FILTERMODE") {
        mFilter = (FilterMode) std::stoi(params[0]);
        return true;
    }
    return false;
}

/**
 * Returns all keys of data saved in this object
 * BaseInfo adds no keys
 * @return set of strings of keys
 */
std::set<std::string> BaseInfo::getKeys() const {
    return {};
}