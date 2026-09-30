#pragma once
#include <cstdlib>
#include <cstring>

// Player-only test feature. Keep spell state and all non-casting combat gates.
namespace MeleeCasting {
inline bool Enabled() {
    static const bool enabled = [] {
        const char* value = std::getenv("EQDREAM_TEST_MELEE_CASTING");
        return value && std::strcmp(value, "1") == 0;
    }();
    return enabled;
}
}
