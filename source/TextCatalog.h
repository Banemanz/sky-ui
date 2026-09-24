#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <sstream>
#include <cstdint>

// Resource parser independent of the game's text encoding and filesystem APIs.
class SkyTextCatalog {
public:
    std::unordered_map<std::string, std::u32string> entries;
    static bool Decode(std::string_view s, std::u32string& out) {
        out.clear();
        for (size_t i = 0; i < s.size();) {
            uint32_t c = static_cast<unsigned char>(s[i++]);
            unsigned n = 0; uint32_t minimum = 0;
            if (c >= 0xC2 && c <= 0xDF) { c &= 31; n = 1; minimum = 0x80; }
            else if (c >= 0xE0 && c <= 0xEF) { c &= 15; n = 2; minimum = 0x800; }
            else if (c >= 0xF0 && c <= 0xF4) { c &= 7; n = 3; minimum = 0x10000; }
            else if (c >= 0x80) return false;
            if (i + n > s.size()) return false;
            while (n--) {
                auto b = static_cast<unsigned char>(s[i++]);
                if ((b & 0xC0) != 0x80) return false;
                c = (c << 6) | (b & 63);
            }
            if (c < minimum || c > 0x10FFFF || (c >= 0xD800 && c <= 0xDFFF) || c == 0) return false;
            out.push_back(c);
        }
        return true;
    }
    void Load(std::istream& input) {
        std::string line, key;
        bool first = true;
        while (std::getline(input, line)) {
            if (first && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);
            first = false;
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == '#' || line[0] == '/') continue;
            if (line[0] == '[') {
                key.clear();
                if (line.size() > 2 && line.back() == ']') {
                    auto candidate = line.substr(1, line.size() - 2);
                    if (candidate.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_") == std::string::npos)
                        key = candidate;
                }
                continue;
            }
            if (key.empty()) continue;
            std::u32string value;
            if (Decode(line, value)) entries[key] = std::move(value);
            key.clear(); // Exactly one value per key, never re-map previous glyphs.
        }
    }
};
