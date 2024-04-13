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

#ifndef LPE_LIVEPRESETSEXTENSION_H
#define LPE_LIVEPRESETSEXTENSION_H


#include <liblpe/data/models/CommandList.h>
#include <liblpe/controller/LivePresetsController.h>
#include <liblpe/data/LivePresetsModel.h>
#include <liblpe/data/models/base/PluginRecallStrategies.h>
#include <liblpe/controller/AboutController.h>
#include <liblpe/plugins/lpe_ultimate.h>
#include <liblpe/util/ProjectChangeListener.h>

/**
 * LPE = LivePresetsExtension, main class that manages the base classes statically
 */
class LPE {
public:
    LPE(REAPER_PLUGIN_HINSTANCE hInstance, HWND mainHwnd);

    ReaProject* mProject = nullptr;
    LivePresetsModel* mModel = nullptr;
    std::map<ReaProject*, LivePresetsModel> mModels;
    PluginRecallStrategies mPrs;
    LivePresetsController mController;
    ControlViewController mControlView;
    AboutController mAboutController;
    ProjectChangeListener mChangeListener;

    REAPER_PLUGIN_HINSTANCE mInstance;
    HWND mMainHwnd;
    CommandList mActions;

    static void onMenuClicked(const char* menustr, HMENU menu, int flag);
    static bool isMutedShown();
    bool onActionExecuted(int iCmd, int flag);
    bool onActionExecutedEx(KbdSectionInfo* sec, int cmdId, int val, int valhw, int relmode, HWND hwnd);
    bool recallState(ProjectStateContext* ctx, bool isUndo);
    void saveState(ProjectStateContext* ctx, bool isUndo);
    void resetState(bool isUndo);
    void recallPresetByGuid(int data1, int data2, int data3, HWND data4);
    void onRecallPreset(int val, int valhw, int relmode, HWND hwnd);
    void onProjectChanged(ReaProject* proj);
private:
    void createPreset();
    void updatePreset();
    void editPreset();
    void removePresets();
    void showSettings();
    void toggleMutedTracksVisibility();
    void toggleMainWindow();
    void toggleAboutWindow();
    void toggleControlView();
    void onApplySelectedTrackConfigsToAllPresets();
};

inline std::unique_ptr<LPE> g_lpe;


#endif //LPE_LIVEPRESETSEXTENSION_H