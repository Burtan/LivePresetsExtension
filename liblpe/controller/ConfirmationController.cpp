/******************************************************************************
/ LivePresetsExtension
/
/ A generic dialog to confirm any tasks
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

#include <liblpe/resources/resource.h>
#include <liblpe/controller/ConfirmationController.h>
#ifndef _WIN32
    #include <swell/swell.h>
#endif

ConfirmationController::ConfirmationController(const std::string& title, std::string* name)
        : ModalWindow(IDD_CONFIRM, title.data(), "ConfirmationController", 0), mName(name) {
}

void ConfirmationController::onInitDlg() {
    mResizer.init_item(IDC_NAME, 0.0, 0.0, 1.0, 0.0);
    mResizer.init_item(IDC_SAVE, 0.0, 1.0, 0.0, 1.0);
    mResizer.init_item(IDC_CANCEL, 0.0, 1.0, 0.0, 1.0);

    //fills TextEdits
    SetDlgItemText(mHwnd, IDC_NAME, mName->data());
}

int ConfirmationController::onNotify(WPARAM, LPARAM) {
    return 0;
}

int ConfirmationController::onKey(MSG *msg, int keyState) {
    if (msg->message == WM_KEYDOWN) {
        if (!keyState) {
            switch(msg->wParam) {
                case VK_ESCAPE: {
                    mName->clear();
                    close();
                    return 1;
                }
            }
        }
    }
    return 0;
}

void ConfirmationController::onClose() {}

void ConfirmationController::onCommand(WPARAM wparam, LPARAM) {
    switch (wparam) {
        case IDC_CANCEL:
            mName->clear();
            close();
            break;
        case IDC_SAVE: {
            char buf[512];
            GetDlgItemText(mHwnd, IDC_NAME, buf, sizeof(buf));
            mName->assign(buf);
            close();
            break;
        }
        default: {}
    }
}

LPARAM ConfirmationController::result() {
    return !mName->empty();
}
