#pragma once
#include <algorithm>
#include <cstdint>
namespace SpellMasteryCast {
inline int32_t Apply(int32_t original, int32_t focused, unsigned rank) {
    if (!rank) return std::max(focused, 0);
    const int32_t floor = std::clamp(original, 0, 1000);
    if (original <= 1000) return floor;
    const int64_t reduced = static_cast<int64_t>(focused) - 500 * std::min(rank, 10u);
    return static_cast<int32_t>(std::max<int64_t>(floor, reduced));
}
}
