/******************************************************************************
/ LivePresetsExtension
/
/ Bass class for reaper actions
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


#ifndef LPE_BASECOMMAND_H
#define LPE_BASECOMMAND_H

#ifdef _WIN32
    #include <Windows.h>
#else
    #include <WDL/swell/swell-types.h>
#endif
#include <string>

class BaseCommand {
public:
    typedef unsigned short CommandID;
    typedef std::function<void(int val, int valhw, int relmode, HWND hwnd)> Callback;

    BaseCommand(std::string  name, std::string  desc, Callback callback) : mName(std::move(name)),
            mDesc(std::move(desc)), mCallback(std::move(callback)) {}
    virtual ~BaseCommand() = default;

    [[nodiscard]] CommandID id() const { return mCmdId; }
    virtual void run(int val, int valhw, int relmode, HWND hwnd) = 0;
protected:
    std::string mName;
    std::string mDesc;
    CommandID mCmdId = 0;
    Callback mCallback;
};


#endif //LPE_BASECOMMAND_H