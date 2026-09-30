#pragma once
#include <cstdint>
namespace EQDreamSapper {
constexpr uint32_t Ability = 17794;
constexpr uint32_t FirstSpell = 43020;
constexpr uint32_t LastSpell = 43024;
constexpr bool IsSpell(uint32_t id) { return id >= FirstSpell && id <= LastSpell; }
constexpr int Cap(uint32_t id) {
    constexpr int caps[] = {48, 53, 58, 62, 67};
    return IsSpell(id) ? caps[id - FirstSpell] : 0;
}
constexpr bool StealthReady(bool rogue, bool hidden, bool sneaking) {
    return rogue && hidden && sneaking;
}
}
