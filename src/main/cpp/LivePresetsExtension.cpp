/******************************************************************************
/ LivePresetsExtension
/
/ Base extension class
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

#include <sstream>
#include <functional>
#include <LivePresetsExtension.h>
#include <plugins/lpe_ultimate.h>
#include <plugins/reaper_plugin_functions.h>
#include <data/models/HotkeyCommand.h>
#include <data/models/ActionCommand.h>
#include <ui/LivePresetsListAdapter.h>
#include <util/util.h>

/*
Main entry point, is called when the extension is loaded.
*/
LPE::LPE(REAPER_PLUGIN_HINSTANCE hInstance, HWND mainHwnd) : mMainHwnd(mainHwnd), mInstance(hInstance) {
    //register commands and project config with reaper
    mActions.add(std::make_unique<HotkeyCommand>(
            "LPE_OPENTOGGLE_MAIN",
            "LPE - Opens/Closes the LivePresetsExtension main window",
            std::bind(&LPE::toggleMainWindow, this)
    ));

    mActions.add(std::make_unique<HotkeyCommand>(
            "LPE_OPENTOGGLE_ABOUT",
            "LPE - Opens/Closes the LivePresetsExtension about window",
            std::bind(&LPE::toggleAboutWindow, this)
    ));

    //create control view action only on ultimate
    if (Licensing_IsUltimate()) {
        mActions.add(std::make_unique<HotkeyCommand>(
                "LPE_OPENTOGGLE_CONTROL",
                "LPE - Opens/Closes the LivePresetsExtension ControlView window",
                std::bind(&LPE::toggleControlView, this)
        ));
    }

    mActions.add(std::make_unique<HotkeyCommand>(
            "LPE_TRACKSAVEALL",
            "LPE - Saves the track data into all presets",
            std::bind(&LPE::onApplySelectedTrackConfigsToAllPresets, this)
    ));

    mActions.add(std::make_unique<HotkeyCommand>(
            "LPE_TOGGLEMUTEDVISIBILITY",
            "LPE - Shows/Hides muted tracks in TCP",
            std::bind(&LPE::toggleMutedTracksVisibility, this)
    ));

    using namespace std::placeholders;
    mActions.add(std::make_unique<ActionCommand>(
            "LPE_SELECTPRESET",
            "LPE - Selects a preset",
            std::bind(&LPE::onRecallPreset, this, _1, _2, _3, _4)
    ));
}

/**
 * another thing you can register is "hookcommand", which you pass a callback:
     NON_API: bool runCommand(int command, int flag);
            register("hookcommand",runCommand);
   note: it's OK to call Main_OnCommand() within your runCommand, however you MUST check for recursion if doing so!
   in fact, any use of this hook should benefit from a simple reentrancy test...
 * @param iCmd
 * @param flag is usually 0 but can sometimes have useful info depending on the message.
 * @return TRUE to eat (process) the command.
 */
bool LPE::onActionExecuted(int iCmd, int) {
    return mActions.run(iCmd);
}

/**
*  you can also register "hookcommand2", which you pass a callback:
NON_API: bool onAction(KbdSectionInfo *sec, int command, int val, int valhw, int relmode, HWND hwnd);
        register("hookcommand2",onAction);
val/valhw are used for actions learned with MIDI/OSC.
* @param sec
* @param cmdId
* @param val val = [0..127] and valhw = -1 for MIDI CC,
* @param valhw valhw >=0 for MIDI pitch or OSC with value = (valhw|val<<7)/16383.0,
* @param relmode absolute(0) or 1/2/3 for relative adjust modes
* @param hwnd
* @return TRUE to eat (process) the command.
*/
bool LPE::onActionExecutedEx(KbdSectionInfo*, int cmdId, int val, int valhw, int relmode, HWND hwnd) {
    return mActions.run(cmdId, val, valhw, relmode, hwnd);
}

/**
* Called when a menu is opened, you can add a custom menu entry
* @param menustr the identifier string of the menu, available strings:
 * "Track control panel context"
 * "Main toolbar"
 * "Main extensions"
* @param menu the menu object on which you can add further menus
* @param flag 0 means new menu, 1 means menu was already created at least once
*/
void LPE::onMenuClicked(const char* menustr, HMENU menu, int flag) {
    if (strcmp(menustr, "Main extensions") == 0 && flag == 0) {
        //show LPE submenu
        HMENU subMenu = CreatePopupMenu();

        MENUITEMINFO mii{};
        mii.fMask |= MIIM_TYPE | MIIM_ID | MIIM_SUBMENU;
        mii.fType |= MFT_STRING;
        mii.cbSize = sizeof(MENUITEMINFO);

        std::string text = "LivePresets";
        mii.dwTypeData = text.data();
        mii.cch = text.size();

        mii.hSubMenu = subMenu;

        InsertMenuItem(menu, 0, true, &mii);

        mii = MENUITEMINFO();
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_TYPE | MIIM_ID;
        mii.fType = MFT_STRING;

        text = "Presets";
        mii.dwTypeData = text.data();
        mii.cch = text.size();

        mii.wID = NamedCommandLookup("_LPE_OPENTOGGLE_MAIN");

        InsertMenuItem(subMenu, 0, true, &mii);

        //show control menu only in ultimate version
        if (Licensing_IsUltimate()) {
            mii = MENUITEMINFO();
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_TYPE | MIIM_ID;
            mii.fType = MFT_STRING;

            text = "Control View";
            mii.dwTypeData = text.data();
            mii.cch = text.size();

            mii.wID = NamedCommandLookup("_LPE_OPENTOGGLE_CONTROL");
            InsertMenuItem(subMenu, 1, true, &mii);
        }

        //seperator
        mii = MENUITEMINFO();
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fType = MFT_SEPARATOR;
        InsertMenuItem(subMenu, 0, false, &mii);

        //show about
        mii = MENUITEMINFO();
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_TYPE | MIIM_ID;
        mii.fType = MFT_STRING;

        text = "About LPE";
        mii.dwTypeData = text.data();
        mii.cch = text.size();

        mii.wID = NamedCommandLookup("_LPE_OPENTOGGLE_ABOUT");
        InsertMenuItem(subMenu, GetMenuItemCount(subMenu), true, &mii);
    }
    if (strcmp(menustr, "Track control panel context") == 0 && flag == 0) {
        HMENU subMenu = CreatePopupMenu();

        MENUITEMINFO mii{};
        mii.fMask |= MIIM_TYPE | MIIM_ID | MIIM_SUBMENU;
        mii.fType |= MFT_STRING;
        mii.cbSize = sizeof(MENUITEMINFO);

        std::string text = "LivePresets";
        mii.dwTypeData = text.data();
        mii.cch = text.size();

        mii.hSubMenu = subMenu;

        InsertMenuItem(menu, GetMenuItemCount(menu), true, &mii);

        MENUITEMINFO smii{};
        smii.fMask |= MIIM_TYPE | MIIM_ID;
        smii.fType |= MFT_STRING;
        smii.cbSize = sizeof(MENUITEMINFO);

        std::string subtext = "Save track settings to all presets";
        smii.dwTypeData = subtext.data();
        smii.cch = subtext.size();

        smii.wID = NamedCommandLookup("_LPE_TRACKSAVEALL");

        InsertMenuItem(subMenu, 0, true, &smii);
    }
}

/*
 * Recall a preset by its GUID which is encoded in all 4 variables
 */
void LPE::recallPresetByGuid(int data1, int data2, int data3, HWND data4) {
    mModel.recallPresetByGuid(IntsToGuid(data1, data2, data3, (long) data4));
}

/**
 * Called when Reaper receives a command to call the action "LPE_SELECT"
 * @param val val/valhw are used for actions learned with MIDI/OSC. val = [0..127] and valhw = -1 for MIDI CC,
 * @param valhw valhw >=0 for MIDI pitch or OSC with value = (valhw|val<<7)/16383.0,
 * @param relmode absolute(0) or 1/2/3 for relative adjust modes
 * @param hwnd
 */
void LPE::onRecallPreset(int val, int valhw, int, HWND) {
    if (valhw == -1) {
        //MIDI CC value
        mModel.recallByValue(val);
    } else {
        //OSC or MIDI pitch value
        //calculate integer as float value (0-16383)
        auto osc = (127 - val) * 128 + (128 - valhw);
        if (osc == 16384) osc = 0;

        mModel.recallByValue(osc);
    }
}

/**
 * Returns if there is a muted track shown in TCP
 */
bool LPE::isMutedShown() {
    bool mutedShown = false;
    for (int i = 0; i < GetNumTracks(); i++) {
        MediaTrack* track = GetTrack(nullptr, i);

        if (GetMediaTrackInfo_Value(track, B_MUTE) != 0) {
            mutedShown = mutedShown || GetMediaTrackInfo_Value(track, B_SHOWINTCP) != 0;
        }
    }
    return mutedShown;
}

/**
 * Hide/Show muted tracks in TCP
 */
void LPE::toggleMutedTracksVisibility() {
    //check if there are muted shown tracks, then hide them otherwise show all

    bool mutedShown = isMutedShown();
    for (int i = 0; i < GetNumTracks(); i++) {
        MediaTrack* track = GetTrack(nullptr, i);

        if (GetMediaTrackInfo_Value(track, B_MUTE) != 0) {
            SetMediaTrackInfo_Value(track, B_SHOWINTCP, !mutedShown);
        }
    }

    TrackList_AdjustWindows(true);
}

/**
 * Saves the current track configs in all presets
 */
void LPE::onApplySelectedTrackConfigsToAllPresets() {
    std::vector<MediaTrack*> selectedTracks;
    int trackCount = CountSelectedTracks(nullptr);
    selectedTracks.reserve(trackCount);
    for (int i = 0; i < trackCount; i++) {
        selectedTracks.push_back(GetSelectedTrack(nullptr, i));
    }

    mModel.onApplySelectedTrackConfigsToAllPresets(selectedTracks);
}

void LPE::toggleMainWindow() {
    mController.toggleVisibility();
}

void LPE::toggleControlView() {
    ControlViewController_ToggleVisibility(&mControlView);
}

void LPE::toggleAboutWindow() {
    mAboutController.toggleVisibility();
}

/***********************************************************************************************************************
 * State functions
 **********************************************************************************************************************/

/*
 * Extension data is read here. Is also called on Undo/Redo to get an old persisted state
 */
bool LPE::recallState(ProjectStateContext* ctx, bool) {
    // Go through all lines until the src part ends
    char buf[4096];
    LineParser lp;
    while (!ctx->GetLine(buf, sizeof(buf)) && !lp.parse(buf)) {

        // objects start with <OBJECTNAME
        auto token = lp.gettoken_str(0);
        if (strcmp(token, "<LIVEPRESETSMODEL") == 0) {
            mModel = LivePresetsModel(ctx);
        }

        // data finished on >
        if (strcmp(buf, ">") == 0)
            break;
    }

    //Update ui after loading data or returning to persisted state via undo/redo
    if (mController.mList) {
        auto adapter = std::make_unique<LivePresetsListAdapter>(&mModel.mPresets);
        mController.mList->setAdapter(move(adapter));
    }

    return true;
}

void LPE::saveState(ProjectStateContext* ctx, bool) {
    WDL_FastString chunk;
    mModel.persist(chunk);

    std::string out;
    std::istringstream ss(chunk.Get());
    while (getline(ss, out)) {
        ctx->AddLine("%s", out.data());
    }
}

void LPE::resetState(bool) {
    //cleaning data from model
    mModel.reset();

    //cleaning data from ui
    mController.reset();
    ControlViewController_Reset(&mControlView);
}