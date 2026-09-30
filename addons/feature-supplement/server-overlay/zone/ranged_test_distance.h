#pragma once
#include <cstdlib>
#include <cstring>
// Explicit test-server opt-in; never enabled by melee casting alone.
inline bool RangedTestIgnoreDistance() {
    static const bool enabled = [] {
        const char* value = std::getenv("EQDREAM_TEST_RANGED_NO_DISTANCE");
        return value && std::strcmp(value, "1") == 0;
    }();
    return enabled;
}
