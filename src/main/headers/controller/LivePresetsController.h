/******************************************************************************
/ LivePresetsExtension
/
/ Main extension window that shows a list of presets and buttons to other menus
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

#ifndef LPE_LIVEPRESETSCONTROLLER_H
#define LPE_LIVEPRESETSCONTROLLER_H

#include "ui/base/DockWindow.h"
#include "ui/base/ListView.h"
#include "data/models/LivePreset.h"
#include "LivePresetEditController.h"

class LivePresetsController : public DockWindow {
public:
	LivePresetsController();
    std::unique_ptr<ListView<LivePreset>> mList = nullptr;

    void reset() const;
    void createPreset() const;
    void updateSelectedPreset() const;
    void removeSelectedPresets() const;
    void editSelectedPreset() const;
    static void showSettings();
protected:
	void onCommand(WPARAM wParam, LPARAM lParam) override;
	void onContextMenu(HMENU menu) override;
	int onKey(MSG* msg, int iKeyState) override;
    int onNotify(WPARAM wParam, LPARAM lParam) override;
    void onClose() override;
    void onInitDlg() override;
private:
    static void recallLivePreset(LivePreset* preset);
    void applyFilterToSelectedTracks(int filterIndex) const;
    static LivePreset * editPreset(LivePreset *preset);
};


#endif //LPE_LIVEPRESETSCONTROLLER_H