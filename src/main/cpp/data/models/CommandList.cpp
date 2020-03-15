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

#include <data/models/CommandList.h>

BaseCommand* CommandList::find(const BaseCommand::CommandID id) const {
    const auto it = mCommands.find(id);
    return it != mCommands.end() ? it->second.get() : nullptr;
}

bool CommandList::run(BaseCommand::CommandID id, int val, int valhw, int relmode, HWND hwnd) const {
    if (BaseCommand* command = find(id)) {
        command->run(val, valhw, relmode, hwnd);
        return true;
    }

    return false;
}

BaseCommand::CommandID CommandList::add(std::unique_ptr<BaseCommand> command) {
    auto id = command->id();
    mCommands.emplace(command->id(), std::move(command));
    return id;
}

void CommandList::remove(BaseCommand::CommandID cmdId) {
    mCommands.erase(cmdId);
}