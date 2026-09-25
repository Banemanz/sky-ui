#pragma once
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

struct SkyMenuOptions {
    bool enabled = true;
    bool displayAspect = true;
    bool showGInputHint = true;
    bool ps2TripSkip = true;
    bool reduceMotion = false;
    int promptMode = 0; // 0 automatic, 1 keyboard/mouse, 2 controller (visual only)
    float aspectOverride = 0.0f;
    float animationSpeed = 1.0f;
    float tabScale = 1.0f;
    float helpScale = 1.0f;
    std::string path;
    static float Number(const char* text, float fallback, float low, float high) {
        char* end = nullptr;
        float value = std::strtof(text, &end);
        if (end == text || !std::isfinite(value)) return fallback;
        while (*end == ' ' || *end == '\t') ++end;
        if (*end && *end != ';') return fallback;
        return std::clamp(value, low, high);
    }
    static float ScaleValue(const char* text) { return Number(text, 1.0f, 0.65f, 1.25f); }
    bool ControllerPrompts(bool active) const { return promptMode == 2 || (promptMode == 0 && active); }
    float Aspect(float width, float height, float game) const {
        const float display = width > 0 && height > 0 ? width / height : 4.0f / 3.0f;
        if (aspectOverride > 0) return aspectOverride;
        // A height-scaled 4:3 canvas otherwise extends outside 5:4/portrait displays.
        return std::max(4.0f / 3.0f, displayAspect || !std::isfinite(game) || game <= 0 ? display : game);
    }
    float Blend(unsigned elapsed, float rate = 0.02f) const {
        if (reduceMotion) return 1.0f;
        return std::clamp(elapsed * rate * animationSpeed, 0.0f, 1.0f);
    }
    void Load(const std::string& filename) {
        path = filename;
        enabled = GetPrivateProfileIntA("MAIN", "bEnable", 1, path.c_str()) != 0;
        displayAspect = GetPrivateProfileIntA("UI", "bForceFixedAspectRatio", 1, path.c_str()) != 0;
        showGInputHint = GetPrivateProfileIntA("UI", "bShowGInputHint", 1, path.c_str()) != 0;
        reduceMotion = GetPrivateProfileIntA("UI", "bReduceMotion", 0, path.c_str()) != 0;
        ps2TripSkip = GetPrivateProfileIntA("HUD", "bPS2TripSkip", 1, path.c_str()) != 0;
        promptMode = std::clamp(int(GetPrivateProfileIntA("UI", "iPromptMode", 0, path.c_str())), 0, 2);
        char text[64];
        GetPrivateProfileStringA("UI", "fMenuAspectRatio", "0", text, sizeof(text), path.c_str());
        aspectOverride = Number(text, 0.0f, 0.0f, 4.0f);
        if (aspectOverride > 0) aspectOverride = std::max(aspectOverride, 1.0f);
        GetPrivateProfileStringA("UI", "fAnimationSpeed", "1", text, sizeof(text), path.c_str());
        animationSpeed = Number(text, 1.0f, 0.25f, 4.0f);
        GetPrivateProfileStringA("UI", "fTabTextScale", "1.0", text, sizeof(text), path.c_str());
        tabScale = ScaleValue(text);
        GetPrivateProfileStringA("UI", "fHelpTextScale", "1.0", text, sizeof(text), path.c_str());
        helpScale = ScaleValue(text);
    }
};
inline SkyMenuOptions skyMenuOptions;
