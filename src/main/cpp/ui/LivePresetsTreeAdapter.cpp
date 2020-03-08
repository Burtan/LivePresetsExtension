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

#include "ui/LivePresetsTreeAdapter.h"
#include "data/models/TrackInfo.h"

LivePresetsTreeAdapter::LivePresetsTreeAdapter(LivePreset *preset) : mPreset(preset) {

}

void LivePresetsTreeAdapter::onAction(HWND hwnd, HTREEITEM hItem) {
    TVITEM qItem{};
    qItem.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_TEXT;
    qItem.hItem = hItem;
    TreeView_GetItem(hwnd, &qItem);
    ItemData data = mData[qItem.lParam];
    switch (data.type) {
        case TYPE::MASTERTRACK: {
            auto* masterTrack = (MasterTrackInfo*) data.lParam;
            masterTrack->shuffleFilter();
            qItem.pszText = masterTrack->getTreeText();
            break;
        }
        case TYPE::TRACK: {
            auto* track = (TrackInfo*) data.lParam;
            track->shuffleFilter();
            qItem.pszText = track->getTreeText();
            break;
        }
        case TYPE::PARAMS: {
            auto* params = (ParameterInfo*) data.lParam;
            params->shuffleFilter();
            qItem.pszText = params->getTreeText();
            break;
        }
        case TYPE::FX: {
            auto* fx = (FxInfo*) data.lParam;
            fx->shuffleFilter();
            qItem.pszText = fx->getTreeText();
            break;
        }
        case TYPE::SEND: {
            auto* send = (BaseSendInfo*) data.lParam;
            send->shuffleFilter();
            qItem.pszText = send->getTreeText();
            break;
        }
        case TYPE::PARAM: {
            auto* param = (Parameter<double>*) data.lParam;
            param->shuffleFilter();
            qItem.pszText = param->getTreeText();
            break;
        }
        case TYPE::LIVEPRESET:
        case TYPE::NOTHING:
            break;
    }
    TreeView_SetItem(hwnd, &qItem);
}

std::vector<TVITEM> LivePresetsTreeAdapter::getChilds(TVITEM* parent) {

    std::vector<TVITEM> childs;
    if (!parent) {
        mData.clear();
        // root node, show tracks
        {
            TVITEM child;
            child.mask = TVIF_TEXT| TVIF_PARAM | TVIF_STATE;

            child.pszText = mPreset->mMasterTrack->getTreeText();
            child.cchTextMax = sizeof(mPreset->mMasterTrack->getTreeText());

            mData.push_back({TYPE::MASTERTRACK, (LPARAM) mPreset->mMasterTrack, TYPE::LIVEPRESET});
            child.lParam = (LPARAM) mData.size() - 1;

            child.state = TVIS_EXPANDED;
            childs.push_back(child);
        }

        for (auto track : mPreset->mTracks) {
            TVITEM child;
            child.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;

            child.pszText = track->getTreeText();
            child.cchTextMax = sizeof(track->getTreeText());

            mData.push_back({TYPE::TRACK, (LPARAM) track, TYPE::LIVEPRESET});
            child.lParam = (LPARAM) mData.size() - 1;

            child.state = TVIS_EXPANDED;
            childs.push_back(child);
        }
    } else {
        ItemData data = mData[parent->lParam];
        switch (data.type) {
            case TYPE::MASTERTRACK: {
                addChildsForMasterTrack((MasterTrackInfo*) data.lParam, &childs);
                break;
            }
            case TYPE::TRACK: {
                addChildsForTrack((TrackInfo*) data.lParam, &childs);
                break;
            }
            case TYPE::PARAMS: {
                if (data.parentType != FX) {
                    addChildsForParams((ParameterInfo*) data.lParam, &childs);
                }
                break;
            }
            case TYPE::FX: {
                addChildsForFx((FxInfo*) data.lParam, &childs);
                break;
            }
            case TYPE::SEND: {
                addChildsForSend((BaseSendInfo*) data.lParam, &childs);
                break;
            }
            case TYPE::NOTHING:
            case TYPE::PARAM:
            case TYPE::LIVEPRESET:
                break;
        }
    }

    return childs;
}

void LivePresetsTreeAdapter::addChildsForTrack(TrackInfo* item, std::vector<TVITEM>* childs) {
    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = item->mName.getTreeText();
        child.cchTextMax = sizeof(item->mName.getTreeText());

        mData.push_back({TYPE::NOTHING, (LPARAM) &item->mName, TYPE::TRACK});
        child.lParam = (LPARAM) mData.size() - 1;

        childs->push_back(child);
    }

    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = item->mParamInfo.getTreeText();
        child.cchTextMax = sizeof(item->mParamInfo.getTreeText());

        mData.push_back({TYPE::PARAMS, (LPARAM) &item->mParamInfo, TYPE::TRACK});
        child.lParam = (LPARAM) mData.size() - 1;

        childs->push_back(child);
    }

    for (auto& fxInfo : item->mRecFxs) {
        TVITEM child;
        child.mask = TVIF_TEXT| TVIF_PARAM;

        child.pszText = fxInfo->getTreeText();
        child.cchTextMax = sizeof(fxInfo->getTreeText());

        mData.push_back({TYPE::FX, (LPARAM) fxInfo, TYPE::TRACK});
        child.lParam = (LPARAM) mData.size() - 1;

        childs->push_back(child);
    }

    for (auto& fxInfo : item->mFxs) {
        TVITEM child;
        child.mask = TVIF_TEXT| TVIF_PARAM;

        child.pszText = fxInfo->getTreeText();
        child.cchTextMax = sizeof(fxInfo->getTreeText());

        mData.push_back({TYPE::FX, (LPARAM) fxInfo, TYPE::TRACK});
        child.lParam = (LPARAM) mData.size() - 1;

        childs->push_back(child);
    }

    for (auto sendInfo : item->mSwSends) {
        addSendInfoChild(sendInfo, childs);
    }

    for (auto sendInfo : item->mHwSends) {
        addSendInfoChild(sendInfo, childs);
    }
}

void LivePresetsTreeAdapter::addChildsForMasterTrack(MasterTrackInfo* item, std::vector<TVITEM>* childs) {
    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = item->mParamInfo.getTreeText();
        child.cchTextMax = sizeof(item->mParamInfo.getTreeText());

        mData.push_back({TYPE::PARAMS, (LPARAM) &item->mParamInfo, TYPE::MASTERTRACK});
        child.lParam = (LPARAM) mData.size() - 1;

        childs->push_back(child);
    }

    for (auto fxInfo : item->mFxs) {
        TVITEM child;
        child.mask = TVIF_TEXT| TVIF_PARAM;

        child.pszText = fxInfo->getTreeText();
        child.cchTextMax = sizeof(fxInfo->getTreeText());

        mData.push_back({TYPE::FX, (LPARAM) fxInfo, TYPE::MASTERTRACK});
        child.lParam = (LPARAM) mData.size() - 1;

        childs->push_back(child);
    }

    for (auto sendInfo : item->mHwSends) {
        addSendInfoChild(sendInfo, childs);
    }
}

void LivePresetsTreeAdapter::addChildsForParams(ParameterInfo* item, std::vector<TVITEM>* childs) {
    for (auto& key : item->getKeys()) {
        const auto& param = item->at(key);
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = (char*) param.getTreeText();
        child.cchTextMax = sizeof(param.getTreeText());

        mData.push_back({TYPE::PARAM, (LPARAM) &param, TYPE::PARAMS});
        child.lParam = (LPARAM) mData.size() - 1;

        childs->push_back(child);
    }
}

void LivePresetsTreeAdapter::addChildsForFx(FxInfo* item, std::vector<TVITEM>* childs) {

    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = (char*) item->mIndex.getTreeText();
        child.cchTextMax = sizeof(item->mIndex.getTreeText());

        mData.push_back({TYPE::NOTHING, (LPARAM) &item->mIndex, TYPE::FX});
        child.lParam = (LPARAM) mData.size() - 1;

        childs->push_back(child);
    }
    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = (char*) item->mEnabled.getTreeText();
        child.cchTextMax = sizeof(item->mEnabled.getTreeText());

        mData.push_back({TYPE::NOTHING, (LPARAM) &item->mEnabled, TYPE::FX});
        child.lParam = (LPARAM) mData.size() - 1;

        childs->push_back(child);
    }
    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = (char*) item->mPresetName.getTreeText();
        child.cchTextMax = sizeof(item->mPresetName.getTreeText());

        mData.push_back({TYPE::NOTHING, (LPARAM) &item->mPresetName, TYPE::FX});
        child.lParam = (LPARAM) mData.size() - 1;

        childs->push_back(child);
    }
    {
        TVITEM child;
        child.mask = TVIF_TEXT | TVIF_PARAM;

        child.pszText = (char*) item->mParamInfo.getTreeText();
        child.cchTextMax = sizeof(item->mParamInfo.getTreeText());

        mData.push_back({TYPE::PARAMS, (LPARAM) &item->mParamInfo, TYPE::FX});
        child.lParam = (LPARAM) mData.size() - 1;

        childs->push_back(child);
    }
}

void LivePresetsTreeAdapter::addChildsForSend(BaseSendInfo* item, std::vector<TVITEM>* childs) {
    TVITEM child;
    child.mask = TVIF_TEXT | TVIF_PARAM;

    child.pszText = item->mParamInfo.getTreeText();
    child.cchTextMax = sizeof(item->mParamInfo.getTreeText());

    mData.push_back({TYPE::PARAMS, (LPARAM) &item->mParamInfo, TYPE::SEND});
    child.lParam = (LPARAM) mData.size() - 1;

    childs->push_back(child);
}

void LivePresetsTreeAdapter::addSendInfoChild(BaseSendInfo *item, std::vector<TVITEM>* childs) {
    TVITEM child;
    child.mask = TVIF_TEXT | TVIF_PARAM;

    child.pszText = item->getTreeText();
    child.cchTextMax = sizeof(item->getTreeText());

    mData.push_back({TYPE::SEND, (LPARAM) item, TYPE::SEND});
    child.lParam = (LPARAM) mData.size() - 1;

    childs->push_back(child);
}



