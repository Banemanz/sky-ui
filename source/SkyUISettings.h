#pragma once
#include "GInputSettings.h"
#include "MenuOptions.h"
#include <cstdio>

// Reuse the panel's navigation only. SkyUI writes its own scoped keys and never
// notifies GInput or installs/removes executable hooks while a game is running.
class SkyUISettings : public GInputSettings {
    SkyMenuOptions& config;
public:
    static constexpr float aspects[] = {0.0f, 4.0f/3.0f, 16.0f/10.0f, 16.0f/9.0f, 21.0f/9.0f, 32.0f/9.0f};
    explicit SkyUISettings(SkyMenuOptions& optionsConfig) : config(optionsConfig) {
        path = L"SkyUI"; // Availability marker; writes use config.path.
        options = {
            {"UI", "iPromptMode", "Button prompts", 0, 0, 2, 1},
            {"UI", "fTabTextScale", "Tab text size (%)", 100, 65, 125, 5},
            {"UI", "fHelpTextScale", "Help text size (%)", 100, 65, 125, 5},
            {"UI", "bForceFixedAspectRatio", "Use display aspect ratio", 1, 0, 1, 1},
            {"UI", "fMenuAspectRatio", "Menu aspect preset", 0, 0, 5, 1},
            {"UI", "bShowGInputHint", "Show GInput settings link", 1, 0, 1, 1},
#ifndef GTASA
            {"UI", "bReduceMotion", "Reduce menu motion", 0, 0, 1, 1},
            {"UI", "fAnimationSpeed", "Animation speed (%)", 100, 25, 400, 25},
#endif
        };
    }
    int Read(size_t i, int = 0) const override {
        const std::string key = options.at(i).key;
        if (key == "iPromptMode") return config.promptMode;
        if (key == "fTabTextScale") return int(std::lround(config.tabScale * 100));
        if (key == "fHelpTextScale") return int(std::lround(config.helpScale * 100));
        if (key == "bForceFixedAspectRatio") return config.displayAspect;
        if (key == "bShowGInputHint") return config.showGInputHint;
        if (key == "bReduceMotion") return config.reduceMotion;
        if (key == "fAnimationSpeed") return int(std::lround(config.animationSpeed * 100));
        for (int j = 0; j < 6; ++j) if (std::abs(config.aspectOverride - aspects[j]) < 0.0001f) return j;
        return 6; // Preserve a custom INI ratio until the user explicitly edits it.
    }
    std::string ValueLabel(size_t i) const override {
        const std::string key = options.at(i).key;
        const int value = Read(i);
        if (key == "iPromptMode") return value == 0 ? "Automatic" : value == 1 ? "Keyboard / mouse" : "Controller";
        if (key == "fMenuAspectRatio") {
            const char* labels[] = {"Automatic", "4:3", "16:10", "16:9", "21:9", "32:9", "Custom INI"};
            return labels[std::clamp(value, 0, 6)];
        }
        if (options[i].max == 1) return value ? "On" : "Off";
        return std::to_string(value) + "%";
    }
    bool Write(size_t i, int value, int = 0) override {
        if (i >= options.size() || config.path.empty()) { status = "SkyUI INI unavailable"; return false; }
        const auto& option = options[i];
        value = std::clamp(value, option.min, option.max);
        if (Read(i) == value) return false;
        float number = float(value);
        const std::string key = option.key;
        if (key == "fMenuAspectRatio") number = aspects[value];
        else if (key == "fTabTextScale" || key == "fHelpTextScale" || key == "fAnimationSpeed") number /= 100.0f;
        // Locale-independent decimal formatting: do not persist a decimal comma.
        std::string text = std::to_string(value);
        if (key[0] == 'f') {
            const int millionths = int(std::lround(number * 1000000.0f));
            text = std::to_string(millionths / 1000000) + ".";
            const auto fraction = std::to_string(millionths % 1000000);
            text += std::string(6 - fraction.size(), '0') + fraction;
        }
        if (!WritePrivateProfileStringA("UI", option.key, text.c_str(), config.path.c_str())) {
            status = "Could not save SkyUI INI";
            return false; // Leave the live value unchanged on a failed write.
        }
        if (key == "iPromptMode") config.promptMode = value;
        else if (key == "fTabTextScale") config.tabScale = number;
        else if (key == "fHelpTextScale") config.helpScale = number;
        else if (key == "bForceFixedAspectRatio") config.displayAspect = value != 0;
        else if (key == "bShowGInputHint") config.showGInputHint = value != 0;
        else if (key == "bReduceMotion") config.reduceMotion = value != 0;
        else if (key == "fAnimationSpeed") config.animationSpeed = number;
        else config.aspectOverride = number;
        status = "Saved - applied";
        return true;
    }
    bool Change(int direction) override {
        if (!direction || selection < 0 || size_t(selection) >= options.size()) return false;
        if (std::string(options[selection].key) == "fMenuAspectRatio") {
            const int current = Read(selection);
            const int next = direction < 0 ? (current + 5) % 6 : (current + 1) % 6;
            return Write(selection, current == 6 ? (direction < 0 ? 5 : 0) : next);
        }
        return GInputSettings::Change(direction);
    }
    bool RestoreSelected() {
        return selection >= 0 && size_t(selection) < options.size() && Write(selection, options[selection].fallback);
    }
};
