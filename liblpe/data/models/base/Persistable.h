/******************************************************************************
/ LivePresetsExtension
/
/ Adds base functions for persisting data
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

#ifndef LPE_PERSISTABLE_H
#define LPE_PERSISTABLE_H

#define WDL_NO_DEFINE_MINMAX

#include <string>
#include <vector>
#include <cstring> //needed for WDL/wdlstring
#include <third_party/WDL/WDL/wdlstring.h>
#include <liblpe/plugins/reaper_plugin.h>

class Persistable {
public:
    //persistance
    virtual void persist(WDL_FastString& str) const final;
    virtual void initFromChunk(ProjectStateContext* ctx) final;
protected:
    //persistance
    /**
     * Persist all additional subclass data here
     * @param str the string in which to save all persisting data
     */
    virtual void persistHandler(WDL_FastString& str) const = 0;
    virtual bool initFromChunkHandler(std::string& key, std::vector<const char*>& params) = 0;
    virtual bool initFromChunkHandler(std::string& key, ProjectStateContext* ctx) = 0;
private:
    [[nodiscard]] virtual std::string getChunkId() const = 0;
};


#endif //LPE_PERSISTABLE_H