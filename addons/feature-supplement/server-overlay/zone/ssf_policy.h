#pragma once

// Character-scoped restrictions; never change shared item definitions.
namespace EQDreamSSF {
constexpr const char* Flag = "EQDreamSoloSelfFound";

inline bool Enabled(Client* client) {
    return client && client->GetBucket(Flag) == "1";
}

inline bool Restricted(const EQ::ItemData* item) {
    if (!item) return false;
    if (item->ItemClass != EQ::item::ItemClassCommon) return true;
    switch (item->ItemType) {
        case EQ::item::ItemTypeFood:
        case EQ::item::ItemTypeDrink:
        case EQ::item::ItemTypePotion:
        case EQ::item::ItemTypeAugmentation:
            return false;
        default:
            return item->AugType == 0;
    }
}

inline bool Restricted(const EQ::ItemInstance* inst) {
    if (!inst) return false;
    if (Restricted(inst->GetItem())) return true;
    if (inst->IsClassBag()) {
        for (int i = 0; i < EQ::invbag::SLOT_COUNT; ++i)
            if (Restricted(inst->GetItem(i))) return true;
    }
    return false;
}

inline void Bind(EQ::ItemInstance* inst) {
    if (!inst) return;
    if (Restricted(inst->GetItem())) inst->SetAttuned(true);
    if (inst->IsClassBag()) {
        for (int i = 0; i < EQ::invbag::SLOT_COUNT; ++i) Bind(inst->GetItem(i));
    }
}

inline bool SharedSlot(uint32 slot) {
    return (slot >= EQ::invslot::SHARED_BANK_BEGIN && slot <= EQ::invslot::SHARED_BANK_END) ||
        (slot >= EQ::invbag::SHARED_BANK_BAGS_BEGIN && slot <= EQ::invbag::SHARED_BANK_BAGS_END);
}

inline void Explain(Client* client) {
    client->Message(Chat::Yellow, "Solo self-found: only food, water, potions and augments may be transferred between players.");
}
}
