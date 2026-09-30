#pragma once
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>

namespace NpcManaRecovery {
inline int64_t Pool(int64_t original) {
    if (original <= 0) return original;
    return original > std::numeric_limits<int64_t>::max() / 3
        ? std::numeric_limits<int64_t>::max() : original * 3;
}
inline bool Enabled(unsigned classes) {
    const char* value = std::getenv("EQDREAM_NPC_MANA_RECOVERY_MODULE");
    return classes > 1 && value && std::strcmp(value, "1") == 0;
}
// Minimum total recovery each existing six-second tick, not an extra bonus.
// Preserve larger native regeneration and negative native drains.
inline int64_t Tick(int64_t native, int64_t maximum, bool engaged) {
    if (maximum <= 0 || native < 0) return native;
    const int64_t percent = engaged ? 3 : 10;
    const int64_t floor = (maximum / 100) * percent + ((maximum % 100) * percent) / 100;
    return std::max(native, std::max<int64_t>(1, floor));
}
}
