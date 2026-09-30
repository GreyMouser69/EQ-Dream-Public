#pragma once

#include "client.h"
#include <algorithm>
#include <string>

// Test-only personal spell ranks and the ten saved Spell Shard Forge slots.
// Values are character-scoped data buckets, while Spell Shards are account-wide.
namespace SpellMastery {
inline constexpr const char* BucketPrefix = "SpellMastery_";
inline constexpr const char* SlotBucketPrefix = "SpellMasterySlot_";
inline constexpr uint32 ShardCurrency = 7;
inline constexpr unsigned MaxRank = 10;
inline constexpr unsigned MaxSlots = 10;
inline constexpr uint32 ShardItem = 990101;
inline constexpr uint32 ShardBundleItem = 990102;
// Sum of each purchased rank, not merely the final rank cost.
inline unsigned UpgradeCost(unsigned current, unsigned gained) {
    if (current >= MaxRank || gained == 0) return 0;
    gained = std::min(gained, MaxRank - current);
    return (1u << (current + gained)) - (1u << current);
}
inline constexpr unsigned PowerPercentPerRank = 2;
inline constexpr unsigned DurationPercentPerRank = 2;
inline constexpr unsigned ResistDifficultyPerRank = 40;
inline constexpr unsigned CastTimeReductionMsPerRank = 500;
inline constexpr unsigned MaxCastTimeReductionMs = 5000;

inline std::string BucketKey(uint16 spell_id)
{
    return std::string(BucketPrefix) + std::to_string(spell_id);
}

inline std::string SlotKey(unsigned slot)
{
    return std::string(SlotBucketPrefix) + std::to_string(slot);
}

inline unsigned Rank(Client* client, uint16 spell_id)
{
    if (!client || !IsValidSpell(spell_id)) {
        return 0;
    }
    const auto value = Strings::ToUnsignedInt(client->GetBucket(BucketKey(spell_id)));
    return std::min<unsigned>(value, MaxRank);
}

inline unsigned Rank(Mob* caster, uint16 spell_id)
{
    return caster && caster->IsClient() ? Rank(caster->CastToClient(), spell_id) : 0;
}

inline uint16 SlotSpell(Client* client, unsigned slot)
{
    if (!client || slot < 1 || slot > MaxSlots) {
        return 0;
    }
    const auto spell_id = static_cast<uint16>(Strings::ToUnsignedInt(client->GetBucket(SlotKey(slot))));
    return IsValidSpell(spell_id) ? spell_id : 0;
}

inline void SetSlot(Client* client, unsigned slot, uint16 spell_id)
{
    if (client && slot >= 1 && slot <= MaxSlots) {
        client->SetBucket(SlotKey(slot), std::to_string(spell_id));
    }
}

inline bool IsUpgradeable(Client* client, uint16 spell_id)
{
    if (!client || !IsValidSpell(spell_id) || IsCombatSkill(spell_id) || IsDiscipline(spell_id)) {
        return false;
    }
    const auto scribed = client->GetScribedSpells();
    return std::find(scribed.begin(), scribed.end(), spell_id) != scribed.end();
}
} // namespace SpellMastery
