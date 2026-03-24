#include "menusettings.h"

#include <simpleini/SimpleIni.h>

#include "menucontrols.h"
#include "menukeyboard.h"
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include "menu.h"
#include "input_name_map.h"
#pragma warning(push)
#pragma warning(disable: 4244)
namespace NativeMenu {


    MenuSettings::MenuSettings() { }


    MenuSettings::~MenuSettings() { }

    void MenuSettings::SetFiles(const std::string &menu) {
        settingsMenuFile = menu;
    }

    void MenuSettings::ReadSettings(MenuControls *control, Menu *menuOpts) {
        CSimpleIniA settingsMenu;
        settingsMenu.SetUnicode();
        settingsMenu.LoadFile(settingsMenuFile.c_str());
        control->ControlKeys[MenuControls::ControlType::MenuKey] =
            GetKeyFromName(settingsMenu.GetValue("MENU", "MenuKey", "VK_OEM_4"));
        control->ControlKeys[MenuControls::ControlType::MenuUp] =
            GetKeyFromName(settingsMenu.GetValue("MENU", "MenuUp", "UP"));
        control->ControlKeys[MenuControls::ControlType::MenuDown] =
            GetKeyFromName(settingsMenu.GetValue("MENU", "MenuDown", "DOWN"));
        control->ControlKeys[MenuControls::ControlType::MenuLeft] =
            GetKeyFromName(settingsMenu.GetValue("MENU", "MenuLeft", "LEFT"));
        control->ControlKeys[MenuControls::ControlType::MenuRight] =
            GetKeyFromName(settingsMenu.GetValue("MENU", "MenuRight", "RIGHT"));
        control->ControlKeys[MenuControls::ControlType::MenuSelect] =
            GetKeyFromName(settingsMenu.GetValue("MENU", "MenuSelect", "RETURN"));
        control->ControlKeys[MenuControls::ControlType::MenuCancel] =
            GetKeyFromName(settingsMenu.GetValue("MENU", "MenuCancel", "BACKSPACE"));

        control->ControllerButton1 = settingsMenu.GetLongValue("MENU", "ControllerButton1", -1);
        control->ControllerButton2 = settingsMenu.GetLongValue("MENU", "ControllerButton2", -1);

        // 支持使用游戏动作（INPUT_*）作为第二套按键映射，示例： MenuUp_Action = INPUT_FRONTEND_UP
        // 映射由 input_name_map.h 提供（自动从 enums.h 生成）

        auto parseAction = [&](const char* raw)->int {
            std::string v = raw ? raw : "";
            // trim
            v.erase(v.begin(), std::find_if(v.begin(), v.end(), [](unsigned char ch){ return !std::isspace(ch); }));
            v.erase(std::find_if(v.rbegin(), v.rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), v.end());
            if (v.empty()) return -1;
            // 现在直接使用枚举名，例如 "INPUT_AIM"；不再接受带有 "#sym:" 前缀的写法
            // lookup known names
            auto it = inputNameMap.find(v);
            if (it != inputNameMap.end()) return it->second;
            // try numeric
            try {
                int n = std::stoi(v);
                return n;
            } catch (...) {
                return -1;
            }
        };

        // Read action mappings and register them (non-breaking: keyboard VK mappings remain primary)
        int aMenuKey = parseAction(settingsMenu.GetValue("MENU", "MenuKey_Action", ""));
        if (aMenuKey >= 0) {
            control->NativeControlActions[MenuControls::ControlType::MenuKey] = aMenuKey;
            control->AddNativeControl(static_cast<eControl>(aMenuKey));
        }
        int aUp = parseAction(settingsMenu.GetValue("MENU", "MenuUp_Action", ""));
        if (aUp >= 0) {
            control->NativeControlActions[MenuControls::ControlType::MenuUp] = aUp;
            control->AddNativeControl(static_cast<eControl>(aUp));
        }
        int aDown = parseAction(settingsMenu.GetValue("MENU", "MenuDown_Action", ""));
        if (aDown >= 0) {
            control->NativeControlActions[MenuControls::ControlType::MenuDown] = aDown;
            control->AddNativeControl(static_cast<eControl>(aDown));
        }
        int aLeft = parseAction(settingsMenu.GetValue("MENU", "MenuLeft_Action", ""));
        if (aLeft >= 0) {
            control->NativeControlActions[MenuControls::ControlType::MenuLeft] = aLeft;
            control->AddNativeControl(static_cast<eControl>(aLeft));
        }
        int aRight = parseAction(settingsMenu.GetValue("MENU", "MenuRight_Action", ""));
        if (aRight >= 0) {
            control->NativeControlActions[MenuControls::ControlType::MenuRight] = aRight;
            control->AddNativeControl(static_cast<eControl>(aRight));
        }
        int aSelect = parseAction(settingsMenu.GetValue("MENU", "MenuSelect_Action", ""));
        if (aSelect >= 0) {
            control->NativeControlActions[MenuControls::ControlType::MenuSelect] = aSelect;
            control->AddNativeControl(static_cast<eControl>(aSelect));
        }
        int aCancel = parseAction(settingsMenu.GetValue("MENU", "MenuCancel_Action", ""));
        if (aCancel >= 0) {
            control->NativeControlActions[MenuControls::ControlType::MenuCancel] = aCancel;
            control->AddNativeControl(static_cast<eControl>(aCancel));
        }

        menuOpts->cheatString = settingsMenu.GetValue("MENU", "CheatString", "");

        menuOpts->menuX = settingsMenu.GetDoubleValue("MENU", "MenuX", 0.0);
        menuOpts->menuY = settingsMenu.GetDoubleValue("MENU", "MenuY", 0.0);

        menuOpts->useSmoothScroll = settingsMenu.GetBoolValue("Navigation", "Smooth Scrolling", false);
        // 兼容：保留旧的 Smooth Factor 配置
        menuOpts->smoothFactor = settingsMenu.GetDoubleValue("Navigation", "Smooth Factor", 0.00001);
        // 新增：选择平滑方式（"factor" 或 "time"），默认使用 factor
        std::string smoothMode = settingsMenu.GetValue("Navigation", "Smooth Mode", "factor");
        menuOpts->smoothUseTimeConstant = (smoothMode == "time");
        // 平滑时间（毫秒），仅当 Smooth Mode = time 时生效，默认 80 ms
        menuOpts->smoothTimeMs = static_cast<float>(settingsMenu.GetDoubleValue("Navigation", "Smooth TimeMs", 80.0));

        // Title Text
        menuOpts->titleTextColor.R =   settingsMenu.GetLongValue("Title Text",   "Red"    , 255);
        menuOpts->titleTextColor.G =   settingsMenu.GetLongValue("Title Text",   "Green", 255);
        menuOpts->titleTextColor.B =   settingsMenu.GetLongValue("Title Text",   "Blue"    , 255);
        menuOpts->titleTextColor.A =   settingsMenu.GetLongValue("Title Text",   "Alpha", 255);
        menuOpts->titleFont =      settingsMenu.GetLongValue("Title Text",   "Font"    , 1);

        menuOpts->titleBackgroundColor.R =   settingsMenu.GetLongValue("Title Rect",   "Red"    , 255);
        menuOpts->titleBackgroundColor.G =   settingsMenu.GetLongValue("Title Rect",   "Green", 255);
        menuOpts->titleBackgroundColor.B =   settingsMenu.GetLongValue("Title Rect",   "Blue"    , 255);
        menuOpts->titleBackgroundColor.A =   settingsMenu.GetLongValue("Title Rect",   "Alpha", 255);
        
        menuOpts->optionsBackgroundSelectColor.R =      settingsMenu.GetLongValue("Scroller",     "Red"    , 255);
        menuOpts->optionsBackgroundSelectColor.G =      settingsMenu.GetLongValue("Scroller",     "Green", 255);
        menuOpts->optionsBackgroundSelectColor.B =      settingsMenu.GetLongValue("Scroller",     "Blue"    , 255);
        menuOpts->optionsBackgroundSelectColor.A =      settingsMenu.GetLongValue("Scroller",     "Alpha", 255);
        
        menuOpts->optionsTextColor.R =      settingsMenu.GetLongValue("Options Text", "Red"    , 255);
        menuOpts->optionsTextColor.G =      settingsMenu.GetLongValue("Options Text", "Green", 255);
        menuOpts->optionsTextColor.B =      settingsMenu.GetLongValue("Options Text", "Blue"    , 255);
        menuOpts->optionsTextColor.A =      settingsMenu.GetLongValue("Options Text", "Alpha", 255);
        menuOpts->optionsFont =   settingsMenu.GetLongValue("Options Text", "Font"    , 0);
        
        menuOpts->optionsBackgroundColor.R = settingsMenu.GetLongValue("Options Rect", "Red"    , 0);
        menuOpts->optionsBackgroundColor.G = settingsMenu.GetLongValue("Options Rect", "Green", 0);
        menuOpts->optionsBackgroundColor.B = settingsMenu.GetLongValue("Options Rect", "Blue"    , 0);
        menuOpts->optionsBackgroundColor.A = settingsMenu.GetLongValue("Options Rect", "Alpha", 255);
    }

    void MenuSettings::SaveSettings() {
        // Make an issue or do a PR or something when you want to have this implemented...
    }
}
#pragma warning(pop)
