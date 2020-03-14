/******************************************************************************
/ LivePresetsExtension
/
/ Represents a list of reaper actions
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

#ifndef LPE_ACTIONLIST_H
#define LPE_ACTIONLIST_H

#include "data/models/HotkeyCommand.h"
#include "data/models/ActionCommand.h"

class CommandList {
public:
    BaseCommand::CommandID add(std::unique_ptr<BaseCommand> command);
    void remove(BaseCommand::CommandID cmdId);
    bool run(BaseCommand::CommandID id, int val = 0, int valhw = 0, int relmode = 0, HWND hwnd = nullptr) const;
private:
    [[nodiscard]] BaseCommand* find(BaseCommand::CommandID id) const;
    std::map<BaseCommand::CommandID, std::unique_ptr<BaseCommand>> mCommands;
};


#endif //LPE_ACTIONLIST_H