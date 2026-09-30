#pragma once
#include <algorithm>
#include <limits>
#include <cstdint>
namespace SpellMasteryPet {
inline unsigned LevelBonus(unsigned rank) {
    rank = std::min(rank, 10u);
    return rank + (rank > 5 ? rank - 5 : 0);
}
// Input must be a fresh focused pet template, never an already-scaled live pet.
template<class T> T Scale(T value, unsigned rank) {
    if (value <= 0 || rank == 0) return value;
    const uint64_t factor = 100 + 50 * std::min(rank, 10u);
    const uint64_t n = static_cast<uint64_t>(value);
    const uint64_t cap = static_cast<uint64_t>(std::numeric_limits<T>::max());
    const uint64_t whole = n / 100;
    if (whole > cap / factor) return std::numeric_limits<T>::max();
    const uint64_t result = whole * factor;
    const uint64_t tail = (n % 100) * factor / 100;
    return static_cast<T>(result > cap - tail ? cap : result + tail);
}
template<class PetStats> void Apply(PetStats& pet, unsigned rank) {
    rank = std::min(rank, 10u);
    if (!rank) return;
    pet.level = static_cast<decltype(pet.level)>(std::min<unsigned>(
        static_cast<unsigned>(pet.level) + LevelBonus(rank), std::numeric_limits<decltype(pet.level)>::max()));
    pet.max_hp = Scale(pet.max_hp, rank);
    pet.current_hp = pet.max_hp;
    pet.min_dmg = Scale(pet.min_dmg, rank);
    pet.max_dmg = Scale(pet.max_dmg, rank);
    pet.AC = Scale(pet.AC, rank);
    pet.STR = Scale(pet.STR, rank);
    pet.STA = Scale(pet.STA, rank);
    pet.DEX = Scale(pet.DEX, rank);
    pet.AGI = Scale(pet.AGI, rank);
    pet.MR = Scale(pet.MR, rank);
    pet.FR = Scale(pet.FR, rank);
    pet.CR = Scale(pet.CR, rank);
    pet.DR = Scale(pet.DR, rank);
    pet.PR = Scale(pet.PR, rank);
}
}
