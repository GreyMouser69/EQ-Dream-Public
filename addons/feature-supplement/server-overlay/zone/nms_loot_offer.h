#pragma once
#include <cstdint>
#pragma pack(push, 1)
struct NMSLootOfferHeader {
	std::uint16_t corpse_id;
	char   looter_name[64];
	std::uint32_t character_id;
	std::uint32_t zone_id;
	std::uint32_t instance_id;
	std::uint32_t reserved;
	std::uint32_t item_count;
};

struct NMSLootOfferItem {
	std::uint32_t item_id;
	std::uint32_t icon;
	std::uint32_t quantity;
	std::uint32_t loot_slot;
	std::uint32_t is_bonus_item;
	std::uint16_t flags;
	char   item_name[64];
	char   source_name[64];
};
#pragma pack(pop)

static_assert(sizeof(NMSLootOfferHeader) == 0x56, "Triune NMS loot header size changed");
static_assert(sizeof(NMSLootOfferItem) == 0x96, "Triune NMS loot item size changed");

