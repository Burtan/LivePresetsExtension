/******************************************************************************
/ LivePresetsExtension
/
/ Entry for the extension
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



#define REAPERAPI_IMPLEMENT
#define REQUIRED_API(name) {reinterpret_cast<void **>(&name), #name, true}

#include <liblpe/LivePresetsExtension.h>
#include <liblpe/plugins/reaper_plugin_functions.h>
#include <thread>
#include <iostream>

static bool loadAPI(void* (*getFunc)(const char*)) {
    struct ApiFunc { void* *ptr; const char* name; bool required; };

    const ApiFunc funcs[] {
            REQUIRED_API(ShowConsoleMsg),
            REQUIRED_API(AddExtensionsMainMenu),
            REQUIRED_API(DoActionShortcutDialog),
            REQUIRED_API(RefreshToolbar),
            REQUIRED_API(NamedCommandLookup),
            REQUIRED_API(GetColorThemeStruct),
            REQUIRED_API(EnsureNotCompletelyOffscreen),
            REQUIRED_API(SectionFromUniqueID),
            REQUIRED_API(PreventUIRefresh),
            REQUIRED_API(Undo_EndBlock),
            REQUIRED_API(Undo_BeginBlock),
            REQUIRED_API(Undo_OnStateChangeEx2),
            REQUIRED_API(plugin_register),
            REQUIRED_API(get_ini_file),
            REQUIRED_API(GetResourcePath),
            REQUIRED_API(genGuid),
            REQUIRED_API(stringToGuid),
            REQUIRED_API(guidToString),
            REQUIRED_API(screenset_unregister),
            REQUIRED_API(screenset_registerNew),
            REQUIRED_API(Dock_UpdateDockID),
            REQUIRED_API(DockWindowRemove),
            REQUIRED_API(DockIsChildOfDock),
            REQUIRED_API(DockWindowActivate),
            REQUIRED_API(DockWindowAddEx),
            REQUIRED_API(GetMasterTrack),
            REQUIRED_API(GetSelectedTrack),
            REQUIRED_API(GetTrackNumSends),
            REQUIRED_API(GetMediaTrackInfo_Value),
            REQUIRED_API(SetMediaTrackInfo_Value),
            REQUIRED_API(SetTrackStateChunk),
            REQUIRED_API(GetTrackStateChunk),
            REQUIRED_API(GetTrackSendInfo_Value),
            REQUIRED_API(SetTrackSendInfo_Value),
            REQUIRED_API(RemoveTrackSend),
            REQUIRED_API(CreateTrackSend),
            REQUIRED_API(GetTrackGUID),
            REQUIRED_API(GetNumTracks),
            REQUIRED_API(GetTrack),
            REQUIRED_API(CountTracks),
            REQUIRED_API(CountSelectedTracks),
            REQUIRED_API(GetTrackName),
            REQUIRED_API(TrackFX_CopyToTrack),
            REQUIRED_API(TrackFX_GetCount),
            REQUIRED_API(TrackFX_GetRecCount),
            REQUIRED_API(TrackFX_GetFXName),
            REQUIRED_API(TrackFX_GetPreset),
            REQUIRED_API(TrackFX_SetPreset),
            REQUIRED_API(TrackFX_GetEnabled),
            REQUIRED_API(TrackFX_SetEnabled),
            REQUIRED_API(TrackFX_GetNumParams),
            REQUIRED_API(TrackFX_GetParam),
            REQUIRED_API(TrackFX_SetParam),
            REQUIRED_API(TrackFX_GetFXGUID),
            REQUIRED_API(TrackList_AdjustWindows),
            REQUIRED_API(GetLastTouchedFX),
            REQUIRED_API(GetMediaTrackInfo_Value),
            REQUIRED_API(GetCurrentProjectInLoadSave)
    };

    for (const ApiFunc &func : funcs) {
        *func.ptr = getFunc(func.name);

        if (func.required && *func.ptr == nullptr) {
            return false;
        }
    }

    return true;
}

/**
 * another thing you can register is "hookcommand", which you pass a callback:
     NON_API: bool runCommand(int command, int flag);
            register("hookcommand",runCommand);
   note: it's OK to call Main_OnCommand() within your runCommand, however you MUST check for recursion if doing so!
   in fact, any use of this hook should benefit from a simple reentrancy tests...
 * @param iCmd
 * @param flag is usually 0 but can sometimes have useful info depending on the message.
 * @return TRUE to eat (process) the command.
 */
static bool hookCommandProc(int iCmd, int flag) {
    return g_lpe->onActionExecuted(iCmd, flag);
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
static bool hookCommand2Proc(KbdSectionInfo* sec, int cmdId, int val, int valhw, int relmode, HWND hwnd) {
    return g_lpe->onActionExecutedEx(sec, cmdId, val, valhw, relmode, hwnd);
}

/**
* Called when a menu is opened, you can add a custom menu entry
* @param menustr the identifier string of the menu, available strings:
 * "Track control panel context"
 * "Main toolbar"
 * "Main extensions"
* @param hMenu the menu object on which you can add further menus
* @param flag
*/
static void hookMenuProc(const char* menustr, HMENU hMenu, int flag) {
    g_lpe->onMenuClicked(menustr, hMenu, flag);
}

/**
 * Tells reaper whether actions can be toggled on and off and in which state they are. Custom menus get checked/unchecked
 * accordingly
 * @param iCmd the action command Id
 * @return -1 = action does not belong to this extension, or does not toggle
 *          0 = action belongs to this extension and is currently set to "off"
 *          1 = action belongs to this extension and is currently set to "on"
 */
static int toggleActionProc(int iCmd) {
    if (iCmd == NamedCommandLookup("_LPE_OPENTOGGLE_MAIN")) {
        return g_lpe->mController.isVisible();
    }
    if (iCmd == NamedCommandLookup("_LPE_TOGGLEMUTEDVISIBILITY")) {
        return LPE::isMutedShown();
    }
    if (iCmd == NamedCommandLookup("_LPE_OPENTOGGLE_CONTROL")) {
        return ControlViewController_IsVisible(&g_lpe->mControlView);
    }
    if (iCmd == NamedCommandLookup("_LPE_OPENTOGGLE_ABOUT")) {
        return g_lpe->mAboutController.isVisible();
    }

    return -1;
}

static project_config_extension_t config = {                                             // NOLINT(cert-err58-cpp)
        /*
         * Extension data is read here. Is also called on Undo/Redo to get an old persisted state
         */
        [](const char* firstLine, ProjectStateContext* ctx, bool isUndo, struct project_config_extension_t*) -> bool {
            // liblpe data is saved in the part starting with <LIVEPRESETS until the consecutive >
            if (strcmp(firstLine, "<LIVEPRESETS") == 0) {
                return g_lpe->recallState(ctx, isUndo);
            } else {
                return false;
            }
        },
        /*
         * Extension data is persisted here
         */
        [](ProjectStateContext* ctx, bool isUndo, struct project_config_extension_t*) -> void {
            //one line in a .rpp file should contain maximal 4096 chars
            ctx->AddLine("<LIVEPRESETS");
            g_lpe->saveState(ctx, isUndo);
            ctx->AddLine(">");
        },
        /*
         * Prepare extension by cleaning up all data
         */
        [](bool isUndo, struct project_config_extension_t*) -> void {
            g_lpe->resetState(isUndo);
        }
};

int extensionInit(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t* pInfo) {
    try {
        plugin_register("projectconfig", &config);
        g_lpe = std::make_unique<LPE>(hInstance, pInfo->hwnd_main);
        plugin_register("csurf_inst", &g_lpe->mChangeListener);
    } catch (...) {
        return 0;
    }
    return 1;
}

int extensionExit() {
    if (plugin_register && g_lpe) {
        g_lpe.release();
        plugin_register("-hookcommand2", (void*) hookCommand2Proc);
        plugin_register("-hookcommand", (void*) hookCommandProc);
        plugin_register("-hookcustommenu", (void*) hookMenuProc);
        plugin_register("-toggleaction", (void *) toggleActionProc);
        plugin_register("-projectconfig", &config);
        plugin_register("-csurf_inst", &g_lpe->mChangeListener);
    }

    return 0; // makes REAPER unloading us
}

/**
 * Main entry point of the extension, all helper functions have to be declared before this function
 */
extern "C" {
    REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInst, reaper_plugin_info_t* rec) {
        if (!rec)
            return extensionExit();
        else if (rec->caller_version != REAPER_PLUGIN_VERSION || !loadAPI(rec->GetFunc))
            return 0;

        if (!extensionInit(hInst, rec))
            return extensionExit();

        std::thread t([](){
            std::cout << "thread function\n";
        });
        std::cout << "main thread\n";
        t.join();

        // hookcommand2 must be registered before hookcommand
        if (!rec->Register("hookcommand2", (void*) hookCommand2Proc))
            return extensionExit();

        if (!rec->Register("hookcommand", (void*) hookCommandProc))
            return extensionExit();

        if (!rec->Register("hookcustommenu", (void*) hookMenuProc))
            return extensionExit();

        if (!rec->Register("toggleaction", (void *) toggleActionProc))
            return extensionExit();

        // makes reaper call the menu hook
        AddExtensionsMainMenu();

        return 1;
    }
}   // end extern C

#ifndef _WIN32
    #include <liblpe/resources/resource.h>
    #include <swell/swell-dlggen.h>
    //#include <liblpe/resources/resource.rc_mac_dlg>
#endif