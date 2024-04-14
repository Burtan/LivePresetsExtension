/******************************************************************************
/ LivePresetsExtension
/
/ Represents a reaper action that can be recalled with osc/midi
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

#include <liblpe/data/models/ActionCommand.h>
#include <liblpe/plugins/reaper_plugin_functions.h>

ActionCommand::ActionCommand(const std::string& name, const std::string& desc, Callback callback)
        : BaseCommand(name, desc, std::move(callback))
{
    custom_action_register_t s{};
    s.idStr = mName.data();
    s.name = mDesc.data();
    s.uniqueSectionId = 0;
    mCmdId = plugin_register("custom_action", (void*) &s);
}

/*
 * Make sure that the associated action is unregistered from reaper
 */
ActionCommand::~ActionCommand() {
    custom_action_register_t s{};
    s.idStr = mName.data();
    s.name = mDesc.data();
    s.uniqueSectionId = 0;
    plugin_register("-custom_action", (void*) &s);
}

void ActionCommand::run(int val, int valhw, int relmode, HWND hwnd) {
    mCallback(val, valhw, relmode, hwnd);
}
