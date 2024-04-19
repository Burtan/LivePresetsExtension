/******************************************************************************
/ LivePresetsExtension
/
/ ProjectStateContext implementation backed up by a simple string
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

#include <liblpe/data/models/StringProjectStateContext.h>

/**
 * Not implemented
 */
void StringProjectStateContext::AddLine(const char *, ...) {}

/**
 * Not implemented
 */
long long int StringProjectStateContext::GetOutputSize() { return 0; }

/**
 *
 * @param buf
 * @param buflen
 * @return
 */
int StringProjectStateContext::GetLine(char* buf, int buflen) {
    auto completeStr = mStr.Get();
    auto restStr = completeStr + mCurrentIndex;
    auto length = strchr(restStr, '\n') - restStr;

    //check that the buffer is big enough
    if (length > buflen) {
        return 0;
    }

    memcpy(buf, restStr, length);
    //terminate c string with 0 at the end
    buf[length] = 0;

    //skip found '\n', so add +1
    mCurrentIndex += length + 1;

    return length == 0;
}

/**
 * Not implemented
 */
int StringProjectStateContext::GetTempFlag() { return 0; }

/**
 * Not implemented
 */
void StringProjectStateContext::SetTempFlag(int) {}

