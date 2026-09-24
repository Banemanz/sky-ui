#pragma once
#include "TextLoader.h"
#include "TextCatalog.h"
#include "EnglishText.h"
#include "CText.h"
#include <fstream>

class SkyPrivateText {
    std::unordered_map<std::string, plugin::string_t> strings;
    bool ready = false;
    void Read(std::istream& input) {
        SkyTextCatalog catalog;
        catalog.Load(input);
        for (const auto& [key, value] : catalog.entries) {
            plugin::string_t encoded;
            bool tag = false;
            for (char32_t code : value) {
                if (code == U'~') tag = !tag;
                // The classic Western font atlas supports Latin-1 through SDK's
                // glyph table. Decode UTF-8 before narrowing SA's byte strings.
                auto c = static_cast<plugin::char_t>(code <= 255 ? code : U'?');
                if (!tag) {
                    auto it = plugin::table.find(c);
                    if (it != plugin::table.end()) c = it->second;
                }
                encoded.push_back(c);
            }
            strings[key] = std::move(encoded);
        }
    }
public:
    // English is also available before menu assets initialize.
    SkyPrivateText() = default;
    void Clear() {
        strings.clear();
        ready = true;
        std::istringstream fallback(SkyEnglishText);
        Read(fallback);
    }
    bool Load(const std::string& path) {
        if (!ready) Clear();
        std::ifstream file(path, std::ios::binary);
        if (!file) return false;
        Read(file);
        return true;
    }
    const plugin::char_t* Get(const char* key) {
        if (!ready) Clear();
        auto it = strings.find(key ? key : "");
        if (it != strings.end()) return it->second.c_str();
        return TheText.Get(key ? key : "");
    }
};
