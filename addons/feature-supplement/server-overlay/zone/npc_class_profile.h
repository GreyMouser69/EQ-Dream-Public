#pragma once
#include <array>
#include <cstdint>
#include <algorithm>

namespace NpcClassModule {
struct Profile {
    std::array<uint8_t, 3> classes{{0, 0, 0}};
    uint8_t count = 0;
};
inline Profile Assign(uint32_t npc_id, uint8_t original, bool boss, bool eligible) {
    Profile result;
    if (!eligible || original < 1 || original > 16) return result;
    result.classes[0] = original;
    result.count = boss ? 3 : 2;
    // Stable by NPC type: respawns and server restarts retain the same classes.
    uint32_t state = npc_id ^ 0x9e3779b9u;
    for (uint8_t slot = 1; slot < result.count; ++slot) {
        state = state * 1664525u + 1013904223u;
        uint8_t candidate = 1 + ((state >> 16) % 16);
        while (std::find(result.classes.begin(), result.classes.begin() + slot, candidate)
               != result.classes.begin() + slot) candidate = candidate % 16 + 1;
        result.classes[slot] = candidate;
    }
    return result;
}
inline int ManaStat(const Profile& p, int intelligence, int wisdom, int charisma) {
    int result = -1;
    for (uint8_t i = 0; i < p.count; ++i) {
        switch (p.classes[i]) {
        case 2: case 3: case 4: case 6: case 10: case 15:
            result = std::max(result, wisdom); break;
        case 5: case 11: case 12: case 13: case 14:
            result = std::max(result, intelligence); break;
        case 8: result = std::max(result, charisma); break;
        default: break;
        }
    }
    return result;
}
inline int64_t Mana(const Profile& p, int level, int intelligence, int wisdom,
                    int charisma, int64_t configured, int64_t bonuses) {
    const int stat = ManaStat(p, intelligence, wisdom, charisma);
    if (stat < 0) return 0;
    const int64_t base = configured > 0 ? configured : (int64_t(stat / 2) + 1) * level;
    return std::max<int64_t>(0, base + bonuses);
}
}
