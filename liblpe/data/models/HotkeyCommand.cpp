/******************************************************************************
/ LivePresetsExtension
/
/ Represents a reaper action that can be triggered with a hotkey
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

#include <liblpe/data/models/HotkeyCommand.h>
#include <reaper_plugin_functions.h>

HotkeyCommand::HotkeyCommand(const std::string& name, const std::string& desc, Callback callback)
        : BaseCommand(name, desc, std::move(callback))
{
    mCmdId = plugin_register("command_id", (void*) mName.data());
    gaccel_register_t g{};
    g.accel.cmd = mCmdId;
    g.desc = mDesc.data();
    plugin_register("gaccel", &g);
}

/*
 * Make sure that the associated action is unregistered from reaper
 */
HotkeyCommand::~HotkeyCommand() {
    mCmdId = plugin_register("-command_id", (void*) mName.data());
    gaccel_register_t g{};
    g.accel.cmd = mCmdId;
    g.desc = mDesc.data();
    plugin_register("-gaccel", &g);
}

void HotkeyCommand::run(int val, int valhw, int relmode, HWND hwnd) {
    mCallback(val, valhw, relmode, hwnd);
}