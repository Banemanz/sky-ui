#pragma once
#include <cstddef>
#include <cstdint>
namespace skyui {
inline bool CopyRgbaToBgra(const uint8_t* src, int width, int height, uint8_t* dst, int stride) {
    if (!src || !dst || width <= 0 || height <= 0 || width > 8192 || height > 8192 || stride < width * 4) return false;
    for (int y = 0; y < height; ++y) {
        const auto* in = src + static_cast<size_t>(y) * width * 4;
        auto* out = dst + static_cast<size_t>(y) * stride;
        for (int x = 0; x < width; ++x) {
            out[4*x] = in[4*x+2]; out[4*x+1] = in[4*x+1];
            out[4*x+2] = in[4*x]; out[4*x+3] = in[4*x+3];
        }
    }
    return true;
}
}
