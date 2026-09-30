#pragma once
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <map>
#include <vector>

namespace NpcDebuffModule {
inline bool Enabled(NPC* npc) {
    const char* value = std::getenv("EQDREAM_NPC_DEBUFF_MODULE");
    return value && std::strcmp(value, "1") == 0 && npc
        && npc->GetAssignedNpcClassCount() > 1;
}
inline bool Eligible(uint16 id) {
    if (!IsValidSpell(id) || IsBeneficialSpell(id) || !IsBuffSpell(id)) return false;
    const auto& s = spells[id];
    if (s.target_type != ST_Target || s.recourse_link || s.short_buff_box
        || s.mana < 0 || s.is_discipline || s.endurance_cost || s.endurance_upkeep) return false;
    bool meaningful = false;
    for (int i = 0; i < EFFECT_COUNT; ++i) {
        const int effect = s.effect_id[i], base = s.base_value[i];
        if (effect == SE_Blank || (effect == SE_CHA && base == 0)) continue;
        // Cure counters describe how a debuff is removed; they are not damage.
        if (effect == SE_DiseaseCounter || effect == SE_PoisonCounter
            || effect == SE_CurseCounter || effect == SE_CorruptionCounter) {
            if (base < 0) return false;
            continue;
        }
        switch (effect) {
        case SE_AttackSpeed:
            if (base <= 0 || base >= 100) return false;
            break;
        case SE_ArmorClass: case SE_ATK: case SE_STR: case SE_DEX:
        case SE_AGI: case SE_STA: case SE_INT: case SE_WIS: case SE_CHA:
        case SE_ResistFire: case SE_ResistCold: case SE_ResistPoison:
        case SE_ResistDisease: case SE_ResistMagic: case SE_ResistAll:
            if (base >= 0) return false;
            break;
        default: return false;
        }
        meaningful = true;
    }
    return meaningful;
}
inline std::vector<uint16> SelectSpells(NPC* npc) {
    struct Choice { uint16 id; int level; };
    std::map<std::vector<int>, Choice> families;
    for (uint8 slot = 1; slot < npc->GetAssignedNpcClassCount(); ++slot) {
        const auto cls = npc->GetAssignedNpcClass(slot);
        if (cls < 1 || cls > 16) continue;
        for (int id = 1; id < SPDAT_RECORDS && id < 65535; ++id) {
            if (!Eligible(id)) continue;
            const auto& s = spells[id];
            const int level = s.classes[cls - 1];
            if (level < 1 || level == 255 || level > npc->GetLevel() || s.mana > npc->GetMaxMana()) continue;
            std::vector<int> key;
            for (int i = 0; i < EFFECT_COUNT; ++i)
                if (s.effect_id[i] != SE_Blank && !(s.effect_id[i] == SE_CHA && s.base_value[i] == 0))
                    key.push_back(s.effect_id[i]);
            std::sort(key.begin(), key.end());
            auto found = families.find(key);
            if (found == families.end() || level > found->second.level) families[key] = {uint16(id), level};
        }
    }
    std::vector<uint16> result;
    for (const auto& entry : families) result.push_back(entry.second.id);
    return result;
}
inline bool AllowedTarget(NPC* caster, Mob* target) {
    if (!target || target == caster || target->IsCorpse() || target->GetHP() <= 0 || target->IsMezzed()) return false;
    auto* owner = caster->GetOwner();
    if (caster->IsCharmed() && !owner) return false;
    if (owner && (target == owner || target->GetOwner() == owner)) return false;
    return caster->CheckAggro(target) && caster->IsAttackAllowed(target, true);
}
}
