/******************************************************************************
/ LivePresetsExtension
/
/ Header to map ultimate classes to the extension
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

#ifndef LPE_ULTIMATE_H
#define LPE_ULTIMATE_H

#ifdef LPE_ULTIMATE
//full headers for ultimate version
#include <controller/ControlViewController.h>
#include <data/models/Hardware.h>
#include <data/models/ControlInfo.h>
#include <data/models/FilterPreset.h>
#include <util/Licensing.h>
#else //LPE_ULTIMATE
//Only include this one in .cpp files for models not in .h files as that creates circular dependencies
//stub classes and functions for ce version
#include <memory>
#include <data/models/base/Persistable.h>
#include <data/models/base/Filterable.h>

class ControlInfo{};
class Hardware{};
class Control{};
class ControlViewController{};
class FilterPreset{};

inline bool Licensing_IsUltimate() { return false; }

inline void ControlViewController_Reset(ControlViewController* controller) {}
inline void ControlViewController_ToggleVisibility(ControlViewController* controller) {}
inline bool ControlViewController_IsVisible(ControlViewController* controller) { return false; }

inline void Hardware_Persist(Hardware* hw, WDL_FastString& str) {}
inline Hardware* Hardware_Create(ProjectStateContext* ctx) { return nullptr; }

inline void ControlInfo_Persist(ControlInfo* info, WDL_FastString& str) {}
inline ControlInfo* ControlInfo_Create(ProjectStateContext* ctx) { return nullptr; }
inline void ControlInfo_RecallSettings(ControlInfo* info, FilterMode filter) {}

inline std::vector<std::string*> FilterPreset_GetNames(std::vector<FilterPreset*>& presets) { return std::vector<std::string*>(); }
inline FilterPreset* FilterPreset_GetFilterByName(const std::vector<FilterPreset*>& presets, std::string* name) { return nullptr; }
inline void FilterPreset_SetName(FilterPreset* preset, const std::string& name) {}
inline void FilterPreset_AddPreset(std::vector<FilterPreset*>& presets, FilterPreset* preset) {}

inline void FilterPreset_Persist(FilterPreset* preset, WDL_FastString& str) {}
inline FilterPreset* FilterPreset_Create(ProjectStateContext* ctx) { return nullptr; }

inline FilterPreset* Parameter_ExtractFilterPreset(void* param) { return nullptr; }
inline bool Parameter_ApplyFilterPreset(void* param, FilterPreset *preset) { return true; }

inline FilterPreset* ControlInfo_ExtractFilterPreset(void* paramInfo) { return nullptr; }
inline bool ControlInfo_ApplyFilterPreset(void* paramInfo, FilterPreset *preset) { return true; }

inline FilterPreset* ParameterInfo_ExtractFilterPreset(void* paramInfo) { return nullptr; }
inline bool ParameterInfo_ApplyFilterPreset(void* paramInfo, FilterPreset *preset) { return true; }

inline FilterPreset* HwSendInfo_ExtractFilterPreset(void* paramInfo) { return nullptr; }
inline bool HwSendInfo_ApplyFilterPreset(void* paramInfo, FilterPreset *preset) { return true; }

inline FilterPreset* SwSendInfo_ExtractFilterPreset(void* paramInfo) { return nullptr; }
inline bool SwSendInfo_ApplyFilterPreset(void* paramInfo, FilterPreset *preset) { return true; }

inline FilterPreset* TrackInfo_ExtractFilterPreset(void* paramInfo) { return nullptr; }
inline bool TrackInfo_ApplyFilterPreset(void* paramInfo, FilterPreset *preset) { return true; }

inline FilterPreset* MasterTrackInfo_ExtractFilterPreset(void* paramInfo) { return nullptr; }
inline bool MasterTrackInfo_ApplyFilterPreset(void* paramInfo, FilterPreset *preset) { return true; }

inline FilterPreset* FxInfo_ExtractFilterPreset(void* paramInfo) { return nullptr; }
inline bool FxInfo_ApplyFilterPreset(void* paramInfo, FilterPreset *preset) { return true; }

inline FilterPreset* LivePreset_ExtractFilterPreset(void* paramInfo) { return nullptr; }
inline bool LivePreset_ApplyFilterPreset(void* paramInfo, FilterPreset *preset) { return true; }
#endif

#endif //LPE_ULTIMATE_H