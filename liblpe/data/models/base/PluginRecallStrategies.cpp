/******************************************************************************
/ LivePresetsExtension
/
/ Manages how different plugin settings are recalled
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

#include <utility>
#include <data/models/base/PluginRecallStrategies.h>
#include <plugins/reaper_plugin_functions.h>

PluginRecallStrategies::PluginRecallStrategies() {
    auto resPath = std::string(GetResourcePath()) + "/LPE_plugin_recall_strategies.ini";

    mVersion = GetPrivateProfileInt("general", "version", 0, resPath.data());
    mDefaultStrategy = (PluginRecallStrategy) GetPrivateProfileInt(
            "general",
            "default",
            (int) PluginRecallStrategy::PRESET,
            resPath.data()
    );

    char buf[65536];
    GetPrivateProfileSection("strategies", buf, sizeof(buf), resPath.data());

    char* curStr = buf;

    while (strlen(curStr) > 0) {
        try {
            auto str = std::string(curStr);
            auto index = str.find('=');

            auto name = str.substr(0, index);
            auto strategy = (PluginRecallStrategy) std::stoi(str.substr(index + 1, index + 2));
            auto entry = std::pair<std::string, PluginRecallStrategy>(name, strategy);
            mStrategies.insert(entry);
        } catch (std::exception&) {}

        curStr = curStr + strlen(curStr) + 1;
    }

    init();
}

void PluginRecallStrategies::write() {
    auto resPath = std::string(GetResourcePath()) + "/LPE_plugin_recall_strategies.ini";

    auto version = std::to_string(mVersion);
    auto defaultStrategy = std::to_string(mDefaultStrategy);
    WritePrivateProfileString("general", "version", version.data(), resPath.data());
    WritePrivateProfileString("general", "default", defaultStrategy.data(), resPath.data());

    for (const auto& pair : mStrategies) {
        auto strategy = std::to_string(pair.second);
        WritePrivateProfileString("strategies", pair.first.data(), strategy.data(), resPath.data());
    }
}

void PluginRecallStrategies::init() {
    while (mVersion < VERSION) {
        switch (mVersion) {
            case 0: initVersion1();
        }
    }
}

void PluginRecallStrategies::initVersion1() {
    mStrategies.insert(std::pair<std::string, PluginRecallStrategy>("Kontakt", PARAMETERS));
    mStrategies.insert(std::pair<std::string, PluginRecallStrategy>("JS: MIDI", PARAMETERS));
    mStrategies.insert(std::pair<std::string, PluginRecallStrategy>("CFX", NONE));

    mVersion++;
}

PluginRecallStrategies::PluginRecallStrategy PluginRecallStrategies::get(const char* plugin) {
    for (const auto& pair : mStrategies) {
        if (strstr(plugin, pair.first.data())) {
            return pair.second;
        }
    }

    return mDefaultStrategy;
}

PluginRecallStrategies::~PluginRecallStrategies() {
    write();
}

