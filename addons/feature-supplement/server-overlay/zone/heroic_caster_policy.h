#pragma once
#include <algorithm>
#include <cstdint>
#include <limits>
namespace HeroicCaster {
constexpr unsigned Classes = 15906; // CLR DRU SHM NEC WIZ MAG ENC
inline int64_t Total(int32_t intelligence, int32_t wisdom) {
    return std::max<int64_t>(0, intelligence) + std::max<int64_t>(0, wisdom);
}
// Average heroic INT and WIS without truncating odd combined totals.
inline double Crit(int64_t total) { return (static_cast<double>(total) / 2.0) * 0.25; }
inline int64_t Scale(int64_t damage, int64_t total) {
    if (damage >= 0 || total <= 0) return damage;
    const long double result = static_cast<long double>(damage) * (100.0L + (total / 2.0L) * 0.5L) / 100.0L;
    if (result <= static_cast<long double>(std::numeric_limits<int64_t>::min()))
        return std::numeric_limits<int64_t>::min();
    return static_cast<int64_t>(result);
}
}
