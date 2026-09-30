#pragma once
#include <cstdlib>
#include <cstring>

namespace NpcPetModule {
inline bool Enabled(Mob* mob) {
    const char* value = std::getenv("EQDREAM_NPC_PET_MODULE");
    return value && std::strcmp(value, "1") == 0 && mob && mob->IsNPC()
        && mob->CastToNPC()->GetAssignedNpcClassCount() != 0;
}
inline bool CanSummon(Mob* mob) {
    return !mob->IsCharmed() && !mob->IsPet() && mob->GetAllPets().empty();
}
template<typename Validator>
inline uint16 SelectSpell(NPC* npc, Validator available) {
    // Prefer the original class, then secondary classes in their stable order.
    for (uint8 slot = 0; slot < npc->GetAssignedNpcClassCount(); ++slot) {
        const uint8 cls = npc->GetAssignedNpcClass(slot);
        if (cls < 1 || cls > 16) continue;
        uint16 best = 0;
        int best_level = -1;
        for (int id = 1; id < SPDAT_RECORDS && id < 65535; ++id) {
            if (!IsValidSpell(id)) continue;
            const int required = spells[id].classes[cls - 1];
            if (required < 1 || required == 255 || required > npc->GetLevel()) continue;
            if (!IsEffectInSpell(id, SE_SummonPet) && !IsEffectInSpell(id, SE_NecPet)
                && !IsEffectInSpell(id, SE_SummonBSTPet)) continue;
            if (!spells[id].teleport_zone[0]) continue;
            if (spells[id].mana > npc->GetMaxMana()) continue;
            if (!available(uint16(id))) continue;
            if (required > best_level) { best = id; best_level = required; }
        }
        if (best) return best;
    }
    return 0;
}
}
