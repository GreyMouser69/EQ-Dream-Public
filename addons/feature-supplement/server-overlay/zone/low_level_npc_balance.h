#pragma once
#include <cstdint>

namespace LowLevelNpcBalance {
inline bool Applies(unsigned level) { return level >= 1 && level <= 20; }
// Positive integer game values round down. Preserve zero and negative drains.
inline int64_t Half(int64_t value, unsigned level) {
    return Applies(level) && value > 0 ? value / 2 : value;
}
}
