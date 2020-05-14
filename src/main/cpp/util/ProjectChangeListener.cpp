/******************************************************************************
/ LivePresetsExtension
/
/ Listens for changes, especially changing projects
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

#include <LivePresetsExtension.h>
#include <util/ProjectChangeListener.h>

const char *ProjectChangeListener::GetTypeString() {
    return "";
}

const char *ProjectChangeListener::GetDescString() {
    return "";
}

const char *ProjectChangeListener::GetConfigString() {
    return "";
}

void ProjectChangeListener::SetTrackListChange() {
    auto val = (long long) GetMediaTrackInfo_Value(GetMasterTrack(nullptr), "P_PROJECT");
    auto *proj = (ReaProject*) val;
    if (g_lpe->mProject != proj) {
        g_lpe->onProjectChanged(proj);
    }
}