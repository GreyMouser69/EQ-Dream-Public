#pragma once
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <map>
#include <vector>

namespace NpcDamageModule {
inline bool Enabled(NPC* npc) {
    const char* value = std::getenv("EQDREAM_NPC_DAMAGE_MODULE");
    return value && std::strcmp(value, "1") == 0 && npc
        && npc->GetAssignedNpcClassCount() > 1;
}
inline bool Eligible(uint16 id) {
    if (!IsValidSpell(id) || IsBeneficialSpell(id) || IsBuffSpell(id)) return false;
    const auto& s = spells[id];
    if (s.target_type != ST_Target || s.cast_restriction || s.recourse_link || s.short_buff_box
        || s.mana < 0 || s.is_discipline || s.endurance_cost || s.endurance_upkeep) return false;
    bool damage = false;
    for (int i = 0; i < EFFECT_COUNT; ++i) {
        if (s.effect_id[i] == SE_Blank || (s.effect_id[i] == SE_CHA && s.base_value[i] == 0)) continue;
        if ((s.effect_id[i] != SE_CurrentHP && s.effect_id[i] != SE_CurrentHPOnce)
            || s.base_value[i] >= 0) return false;
        damage = true;
    }
    return damage;
}
inline std::vector<uint16> SelectSpells(NPC* npc) {
    struct Choice { uint16 id; int level; };
    std::map<int, Choice> elements;
    for (uint8 slot = 1; slot < npc->GetAssignedNpcClassCount(); ++slot) {
        const auto cls = npc->GetAssignedNpcClass(slot);
        if (cls < 1 || cls > 16) continue;
        for (int id = 1; id < SPDAT_RECORDS && id < 65535; ++id) {
            if (!Eligible(id)) continue;
            const auto& s = spells[id];
            const int level = s.classes[cls - 1];
            if (level < 1 || level == 255 || level > npc->GetLevel() || s.mana > npc->GetMaxMana()) continue;
            auto found = elements.find(s.resist_type);
            if (found == elements.end() || level > found->second.level)
                elements[s.resist_type] = {uint16(id), level};
        }
    }
    std::vector<uint16> result;
    for (const auto& entry : elements) result.push_back(entry.second.id);
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
