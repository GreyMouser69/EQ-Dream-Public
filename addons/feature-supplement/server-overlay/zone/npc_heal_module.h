#pragma once
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace NpcHealModule {
constexpr int HealthThreshold = 70;
inline bool Enabled(NPC* npc) {
    const char* value = std::getenv("EQDREAM_NPC_HEAL_MODULE");
    return value && std::strcmp(value, "1") == 0 && npc
        && npc->GetAssignedNpcClassCount() > 1;
}
inline bool Eligible(uint16 id) {
    if (!IsValidSpell(id) || !IsBeneficialSpell(id) || IsBuffSpell(id)) return false;
    const auto& s = spells[id];
    if (s.target_type != ST_Target || s.recourse_link || s.short_buff_box
        || s.mana < 0 || s.is_discipline || s.endurance_cost || s.endurance_upkeep) return false;
    bool heals = false;
    for (int i = 0; i < EFFECT_COUNT; ++i) {
        if (s.effect_id[i] == SE_Blank || (s.effect_id[i] == SE_CHA && s.base_value[i] == 0)) continue;
        if ((s.effect_id[i] != SE_CurrentHP && s.effect_id[i] != SE_CurrentHPOnce)
            || s.base_value[i] <= 0) return false;
        heals = true;
    }
    return heals;
}
inline std::vector<uint16> SelectSpells(NPC* npc) {
    std::vector<uint16> result;
    for (uint8 slot = 1; slot < npc->GetAssignedNpcClassCount(); ++slot) {
        const auto cls = npc->GetAssignedNpcClass(slot);
        if (cls < 1 || cls > 16) continue;
        uint16 best = 0; int best_level = -1;
        for (int id = 1; id < SPDAT_RECORDS && id < 65535; ++id) {
            if (!Eligible(id)) continue;
            const int level = spells[id].classes[cls - 1];
            if (level < 1 || level == 255 || level > npc->GetLevel()
                || spells[id].mana > npc->GetMaxMana()) continue;
            if (level > best_level) { best = uint16(id); best_level = level; }
        }
        if (best && std::find(result.begin(), result.end(), best) == result.end()) result.push_back(best);
    }
    return result;
}
inline bool NeedsHeal(Mob* target) {
    return target && !target->IsCorpse() && target->GetHP() > 0
        && target->GetIntHPRatio() <= HealthThreshold;
}
inline bool AllowedTarget(NPC* caster, Mob* target) {
    if (!NeedsHeal(target)) return false;
    if (target == caster) return true;
    if (caster->IsCharmed()) return caster->GetOwner() == target && target->IsClient();
    if (caster->IsPet() || !target->IsNPC() || target->IsPet() || target->IsCharmed()) return false;
    if (!caster->IsEngaged() || !target->IsEngaged() || !caster->GetPrimaryFaction()) return false;
    if (caster->CheckAggro(target) || target->CheckAggro(caster)
        || target->GetReverseFactionCon(caster) >= FACTION_KINDLY) return false;
    auto* enemy = caster->GetTarget();
    return enemy && enemy != target && target->CheckAggro(enemy);
}
}
