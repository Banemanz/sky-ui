#pragma once
#include <windows.h>
#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

// GInput 1.11 has a read-only settings API. Edit only documented keys in the
// loaded module's existing INI; GInput applies them through its focus reload.
class GInputSettings {
public:
    struct Option { const char* section; const char* key; const char* label; int fallback, min, max, step; };
    std::wstring path;
    std::string status;
    int selection = 0;
    int selectedPad = 1;
    bool active = false;
    std::vector<Option> options = {
#ifdef GTASA
        {"Pad1", "ControlsSet", "Control layout", 1, 1, 2, 1},
#else
        {"Pad1", "ControlsSet", "Control layout", 1, 1, 5, 1},
#endif
        {"GInput", "Vibration", "Vibration", 1, 0, 1, 1},
        #ifdef GTASA
        {"SkyUI", "Pad", "Edit player controller", 1, 1, 2, 1},
#endif
        {"Pad1", "InvertLook", "Invert look", 0, 0, 1, 1},
        {"Pad1", "LeftStickSensitivity", "Left stick sensitivity", 100, 0, 200, 5},
        {"Pad1", "RightStickSensitivity", "Right stick sensitivity", 100, 0, 200, 5},
        {"Pad1", "LeftStickDeadzone", "Left stick deadzone", 24, 0, 95, 1},
        {"Pad1", "RightStickDeadzone", "Right stick deadzone", 27, 0, 95, 1},
        {"Pad1", "InvertLeftXAxis", "Invert left X axis", 0, 0, 1, 1},
        {"Pad1", "InvertLeftYAxis", "Invert left Y axis", 0, 0, 1, 1},
        {"Pad1", "InvertRightXAxis", "Invert right X axis", 0, 0, 1, 1},
        {"Pad1", "InvertRightYAxis", "Invert right Y axis", 0, 0, 1, 1},
        {"Pad1", "SwapLeftAxes", "Swap left axes", 0, 0, 1, 1},
        {"Pad1", "SwapRightAxes", "Swap right axes", 0, 0, 1, 1},
        {"Pad1", "Southpaw", "Southpaw", 0, 0, 1, 1},
        {"Pad1", "SwapSticksDuringAiming", "Swap sticks when aiming", 0, 0, 1, 1},
        {"GInput", "PlayStationButtons", "PlayStation button prompts", 0, 0, 1, 1},
#ifdef GTASA
        {"GInput", "FreeAim", "Free aim", 0, 0, 1, 1},
        {"GInput", "MapPadOneToPadTwo", "First controller to player 2", 0, 0, 1, 1},
#else
        {"Pad1", "SAStyleSniperZoom", "SA-style sniper zoom", 1, 0, 1, 1},
        {"Pad1", "DrivebyWithAnalog", "Drive-by with analog stick", 0, 0, 1, 1},
        {"Pad1", "HotkeyToDriveby", "Drive-by hotkey", 0, 0, 1, 1},
#endif
        {"GInput", "DisableOnFocusLost", "Disable pad when unfocused", 1, 0, 1, 1},
        {"GInput", "CheatsFromPad", "Controller cheats", 1, 0, 1, 1},
        {"GInput", "GuideLaunchesOverlay", "Guide opens Steam overlay", 1, 0, 1, 1},
        {"GInput", "ApplyMissionSpecificFixes", "Mission control fixes", 1, 0, 1, 1},
        {"GInput", "ApplyGXTFixes", "GInput help text fixes", 1, 0, 1, 1},
        {"Pad1", "FaceButtonsSensitivity", "Pressure buttons (SCP)", 50, 1, 100, 5},
        {"Pad1", "SixaxisSensitivity", "Sixaxis sensitivity (SCP)", 150, 1, 300, 5},
        {"Pad1", "SixaxisCarSteering", "Sixaxis car steering", 0, 0, 1, 1},
#ifndef GTA3
        {"Pad1", "SixaxisBikeSteering", "Sixaxis bike steering", 0, 0, 1, 1},
#endif
        {"Pad1", "SixaxisBoatSteering", "Sixaxis boat steering", 0, 0, 1, 1},
        {"Pad1", "SixaxisHeliSteering", "Sixaxis helicopter steering", 0, 0, 1, 1},
        {"Pad1", "SixaxisPlaneSteering", "Sixaxis plane steering", 0, 0, 1, 1},
#ifdef GTASA
        {"Pad1", "SixaxisReloading", "Sixaxis reloading", 1, 0, 1, 1},
        {"Pad1", "SixaxisHydraulics", "Sixaxis hydraulics", 0, 0, 1, 1},
#endif
    };
    static std::wstring Wide(const char* s) { return std::wstring(s, s + strlen(s)); }
    bool Init(HMODULE module) {
        path.clear();
        status.clear();
        wchar_t buffer[32768];
        DWORD n = module ? GetModuleFileNameW(module, buffer, 32768) : 0;
        if (!n || n >= 32768) return false;
        path.assign(buffer, n);
        auto dot = path.find_last_of(L'.');
        if (dot == std::wstring::npos) { path.clear(); return false; }
        path.replace(dot, std::wstring::npos, L".ini");
        if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
            path.clear(); status = "GInput INI missing beside its ASI"; return false;
        }
        return true;
    }
    const char* Section(const Option& o, int padOverride = 0) const {
        return !strcmp(o.section, "Pad1") && (padOverride ? padOverride : selectedPad) == 2 ? "Pad2" : o.section;
    }
    int Read(size_t i, int padOverride = 0) const {
        const auto& o = options.at(i);
        if (!strcmp(o.section, "SkyUI")) return selectedPad;
        return std::clamp(static_cast<int>(GetPrivateProfileIntW(Wide(Section(o, padOverride)).c_str(), Wide(o.key).c_str(), o.fallback, path.c_str())), o.min, o.max);
    }
    std::string ValueLabel(size_t i) const {
        const auto& o = options.at(i);
        const int value = Read(i);
        if (!strcmp(o.key, "ControlsSet")) {
            if (value == o.max) return "GTA IV";
#ifdef GTASA
            return "Classic";
#else
            return "Classic " + std::to_string(value);
#endif
        }
        if (o.min == 0 && o.max == 1) return value ? "On" : "Off";
        return std::to_string(value);
    }
    bool Write(size_t i, int value, int padOverride = 0) {
        if (path.empty()) { status = "GInput settings unavailable"; return false; }
        const auto& o = options.at(i);
        value = std::clamp(value, o.min, o.max);
        if (!strcmp(o.section, "SkyUI")) { selectedPad = value; status.clear(); return true; }
        // Back up once; never replace the user's earlier backup.
        const auto backup = path + L".skyui.bak";
        if (!CopyFileW(path.c_str(), backup.c_str(), TRUE) && GetLastError() != ERROR_FILE_EXISTS) {
            status = "Could not back up GInput INI"; return false;
        }
        if (!WritePrivateProfileStringW(Wide(Section(o, padOverride)).c_str(), Wide(o.key).c_str(), std::to_wstring(value).c_str(), path.c_str())) {
            status = "Could not save GInput INI"; return false;
        }
        status = "Saved";
        return true;
    }
    bool Change(int direction) {
        const auto& o = options.at(selection);
        int value = Read(selection);
        if (o.min == 0 && o.max == 1) value ^= 1;
        else if (selection == 0 || !strcmp(o.section, "SkyUI")) value = value + direction < o.min ? o.max : value + direction > o.max ? o.min : value + direction;
        else value += direction * o.step;
        return Write(selection, value);
    }
};
