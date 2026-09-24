#pragma once
#include <algorithm>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

namespace skyui {
inline int GalleryId(const std::string& name) {
    constexpr const char* prefix = "gallery";
    if (name.size() < 12 || name.compare(0, 7, prefix) || name.compare(name.size() - 4, 4, ".jpg")) return 0;
    int id = 0;
    for (size_t i = 7; i < name.size() - 4; ++i) {
        const int digit = name[i] - '0';
        if (digit < 0 || digit > 9 || id > (std::numeric_limits<int>::max() - digit) / 10) return 0;
        id = id * 10 + digit;
    }
    // Only accept canonical names: deletion must refer to exactly what was indexed.
    return id > 0 && name == "gallery" + std::to_string(id) + ".jpg" ? id : 0;
}
inline std::vector<int> ScanGalleryFiles(const std::filesystem::path& directory) {
    std::vector<int> result;
    std::error_code ec;
    std::filesystem::directory_iterator it(directory, ec), end;
    while (!ec && it != end) {
        if (it->is_regular_file(ec)) {
            const int id = GalleryId(it->path().filename().string());
            if (id && !ec) result.push_back(id);
        }
        it.increment(ec);
    }
    std::sort(result.begin(), result.end());
    return result;
}
}
