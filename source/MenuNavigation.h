#pragma once
#include <cstdint>
#include <cstdlib>

// One event per update: frame hitches never replay a burst of queued edits.
class SkyMenuNavigation {
    int held = 0, stick = 0;
    uint32_t changed = 0;
    bool repeating = false;
public:
    void Reset() { held = stick = 0; changed = 0; repeating = false; }
    int Update(uint32_t now, int x, int y, bool left, bool right, bool up, bool down) {
        // GTA's controller axes are in [-128,128]. Hysteresis filters drift.
        const int ax = std::abs(x), ay = std::abs(y);
        if (ax < 40 && ay < 40) stick = 0;
        else if (ax >= 64 || ay >= 64)
            stick = ax > ay ? (x < 0 ? 1 : 2) : (y < 0 ? 3 : 4);
        int direction = 0;
        if (left != right) direction = left ? 1 : 2;
        else if (up != down) direction = up ? 3 : 4;
        else if (!left && !right && !up && !down) direction = stick;
        if (direction != held) {
            held = direction; changed = now; repeating = false;
            return direction;
        }
        if (!direction) return 0;
        if (uint32_t(now - changed) >= (repeating ? 100u : 400u)) {
            changed = now; repeating = true; return direction;
        }
        return 0;
    }
};
