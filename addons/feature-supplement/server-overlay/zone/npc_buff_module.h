#pragma once
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <map>
#include <vector>

namespace NpcBuffModule {
inline bool Enabled(NPC* npc) {
    const char* value = std::getenv("EQDREAM_NPC_BUFF_MODULE");
    return value && std::strcmp(value, "1") == 0 && npc
        && npc->GetAssignedNpcClassCount() > 1;
}
inline bool Eligible(uint16 id) {
    if (!IsValidSpell(id) || !IsBeneficialSpell(id) || !IsBuffSpell(id)) return false;
    const auto& spell = spells[id];
    if (spell.target_type != ST_Self && spell.target_type != ST_Target) return false;
    if (spell.recourse_link || spell.short_buff_box || spell.mana < 0
        || spell.is_discipline || spell.endurance_cost || spell.endurance_upkeep) return false;
    bool meaningful = false;
    for (int slot = 0; slot < EFFECT_COUNT; ++slot) {
        const int effect = spell.effect_id[slot];
        const int base = spell.base_value[slot];
        if (effect == SE_Blank || (effect == SE_CHA && base == 0)) continue;
        switch (effect) {
        case SE_ArmorClass: case SE_ATK: case SE_STR: case SE_DEX:
        case SE_AGI: case SE_STA: case SE_INT: case SE_WIS: case SE_CHA:
        case SE_TotalHP: case SE_ManaPool: case SE_CurrentMana:
        case SE_ResistFire: case SE_ResistCold: case SE_ResistPoison:
        case SE_ResistDisease: case SE_ResistMagic: case SE_ResistAll:
            if (base < 0) return false;
            break;
        case SE_AttackSpeed:
            if (base < 100) return false;
            break;
        case SE_DamageShield:
            if (base >= 0) return false; // Positive values heal the attacker.
            break;
        default:
            return false; // No heals, procs, transformations, travel, summons or triggers.
        }
        meaningful = true;
    }
    return meaningful;
}
inline std::vector<uint16> SelectSpells(NPC* npc) {
    struct Choice { uint16 id; int level; };
    std::map<std::vector<int>, Choice> families;
    // Native primary-class lists are kept as-is; add only secondary-class buffs.
    for (uint8 slot = 1; slot < npc->GetAssignedNpcClassCount(); ++slot) {
        const auto cls = npc->GetAssignedNpcClass(slot);
        if (cls < 1 || cls > 16) continue;
        for (int id = 1; id < SPDAT_RECORDS && id < 65535; ++id) {
            if (!Eligible(id)) continue;
            const auto& spell = spells[id];
            const int level = spell.classes[cls - 1];
            if (level < 1 || level == 255 || level > npc->GetLevel()
                || spell.mana > npc->GetMaxMana()) continue;
            std::vector<int> key;
            for (int i = 0; i < EFFECT_COUNT; ++i) {
                if (spell.effect_id[i] != SE_Blank
                    && !(spell.effect_id[i] == SE_CHA && spell.base_value[i] == 0))
                    key.push_back(spell.effect_id[i]);
            }
            std::sort(key.begin(), key.end());
            key.push_back(1000 + int(spell.target_type));
            auto found = families.find(key);
            if (found == families.end() || level > found->second.level)
                families[key] = {uint16(id), level};
        }
    }
    std::vector<uint16> result;
    for (const auto& entry : families) result.push_back(entry.second.id);
    return result;
}
inline bool AllowedTarget(NPC* caster, Mob* target) {
    if (!target || target->IsCorpse()) return false;
    if (target == caster) return true;
    return caster->IsCharmed() && caster->GetOwner() == target && target->IsClient();
}
}
