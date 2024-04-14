/******************************************************************************
/ LivePresetsExtension
/
/ TreeAdapter for showing preset's filter options
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

#ifndef LPE_LIVEPRESETSTREEADAPTER_H
#define LPE_LIVEPRESETSTREEADAPTER_H

#ifdef _WIN32
    #include <Windows.h>
    #include <CommCtrl.h>
#else
    #include <third_party/WDL/WDL/swell/swell-types.h>
#endif

#include <map>
#include <memory>
#include <string>
#include <liblpe/plugins/lpe_ultimate.h>
#include <liblpe/ui/base/TreeViewAdapter.h>
#include <liblpe/data/models/LivePreset.h>

class LivePresetsTreeAdapter : public TreeViewAdapter {
public:

    explicit LivePresetsTreeAdapter(LivePreset* preset);

    std::vector<TVITEM> getChilds(TVITEM* parent) override;
    void onAction(HWND hwnd, HTREEITEM qItem) override;
private:
    typedef struct {
        TYPE type;
        LPARAM lParam;
        TYPE parentType;
    } ItemData;

    //used to keep a reference of ItemData that it doesn't leak
    std::map<int, ItemData> mData;
    LivePreset* mPreset;
    void addChildsForMasterTrack(MasterTrackInfo *item, std::vector<TVITEM> *childs);
    void addChildsForTrack(TrackInfo *item, std::vector<TVITEM> *childs);
    void addChildsForParams(ParameterInfo *item, std::vector<TVITEM> *childs);
    void addSendInfoChild(BaseSendInfo *item, std::vector<TVITEM> *childs);
    void addChildsForFx(FxInfo *item, std::vector<TVITEM> *childs);
    void addChildsForSend(BaseSendInfo *item, std::vector<TVITEM> *childs);
};


#endif //LPE_LIVEPRESETSTREEADAPTER_H