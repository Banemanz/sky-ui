#pragma once
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>

struct SkyMenuOptions {
    bool enabled = true;
    bool displayAspect = true;
    float tabScale = 1.0f;
    float helpScale = 1.0f;
    std::string path;
    static float ScaleValue(const char* text) {
        char* end = nullptr;
        float value = std::strtof(text, &end);
        if (end == text || !std::isfinite(value)) return 1.0f;
        while (*end == ' ' || *end == '\t') ++end;
        if (*end && *end != ';') return 1.0f;
        return std::clamp(value, 0.65f, 1.25f);
    }
    void Load(const std::string& filename) {
        path = filename;
        enabled = GetPrivateProfileIntA("MAIN", "bEnable", 1, path.c_str()) != 0;
        displayAspect = GetPrivateProfileIntA("UI", "bForceFixedAspectRatio", 1, path.c_str()) != 0;
        char text[64];
        GetPrivateProfileStringA("UI", "fTabTextScale", "1.0", text, sizeof(text), path.c_str());
        tabScale = ScaleValue(text);
        GetPrivateProfileStringA("UI", "fHelpTextScale", "1.0", text, sizeof(text), path.c_str());
        helpScale = ScaleValue(text);
    }
};
inline SkyMenuOptions skyMenuOptions;
