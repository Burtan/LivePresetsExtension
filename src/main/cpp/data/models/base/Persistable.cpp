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

#include <data/models/base/Persistable.h>
#include <WDL/lineparse.h>

/**
 * Creates the element for persisting this class
 * @param str the string in which to save all persisting data
 */
void Persistable::persist(WDL_FastString& str) const {
    str.Append(("<" + getChunkId() + "\n").data());

    persistHandler(str);

    str.Append(">\n");
}

void Persistable::initFromChunk(ProjectStateContext* ctx) {
    char buf[4096];
    LineParser lp;
    while (!ctx->GetLine(buf, sizeof(buf)) && !lp.parse(buf)) {
        //found identifier
        if (strncmp(lp.gettoken_str(0), "<", 1) == 0) {
            std::string key = lp.gettoken_str(0) + 1;
            if (!initFromChunkHandler(key, ctx))
                fprintf(stderr, "Unhandled element: %s\n", key.data());

        } else if (strcmp(lp.gettoken_str(0), ">") == 0) {
            break;
        } else {
            std::vector<const char*> vals;
            for (int i = 1; i < lp.getnumtokens(); i++) {
                vals.push_back(lp.gettoken_str(i));
            }
            std::string key = lp.gettoken_str(0);
            //default value
            if (vals.empty()) {
                vals.push_back("0");
            }
            //default filter
            if (vals.size() == 1) {
                vals.push_back("0");
            }

            if (!initFromChunkHandler(key, vals))
                fprintf(stderr, "Unhandled attribute: %s\n", key.data());
        }
    }
}