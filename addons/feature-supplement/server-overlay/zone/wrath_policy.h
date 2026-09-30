#pragma once
#include <algorithm>
#include <cstdint>

namespace WrathDreamer {
constexpr unsigned FirstRank = 19040;
constexpr unsigned CasterClasses = 15906; // CLR DRU SHM NEC WIZ MAG ENC, zero-based class mask.
constexpr int Levels[] = {10,20,30,35,40,45,50,55,60,65};
inline unsigned Rank(unsigned owned, unsigned level, unsigned classes) {
    if (!(classes & CasterClasses)) return 0;
    unsigned unlocked = 0;
    for (int required : Levels) if (level >= unsigned(required)) ++unlocked;
    return std::min(owned, unlocked);
}
inline int Power(unsigned rank) { return rank ? 5 + 5 * std::min(rank,10u) : 0; }
inline int Crit(unsigned rank) { return rank >= 3 ? 3 * (std::min(rank,10u)-2) : 0; }
inline int64_t Scale(int64_t value, unsigned rank) {
    // Keep integer rounding consistent with native spell and regeneration calculations.
    return value + value * Power(rank) / 100;
}
}
