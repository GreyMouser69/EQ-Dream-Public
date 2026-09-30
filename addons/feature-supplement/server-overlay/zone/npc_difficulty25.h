#pragma once
#include <limits>
namespace NpcDifficulty25 {
inline bool Eligible(bool enabled, bool npc, bool bot, bool merc, bool owned, bool temporary, bool pet) {
    return enabled && npc && !bot && !merc && !owned && !temporary && !pet;
}
template<typename T> T Scale(T value) {
    if (value <= 0) return value;
    const T bonus = value / 4;
    return value > std::numeric_limits<T>::max() - bonus ? std::numeric_limits<T>::max() : value + bonus;
}
}
