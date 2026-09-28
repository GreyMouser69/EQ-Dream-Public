#ifndef EQEMU_NMS_ANYTHING_STRUCTS_H
#define EQEMU_NMS_ANYTHING_STRUCTS_H

#include "types.h"

namespace NMSAnything {

constexpr uint16 ProtocolVersion = 1;
constexpr uint8 SlotCount = 2;

enum class Action : uint8 {
	Refresh = 0,
	EquipCursor = 1,
	WithdrawCursor = 2
};

enum class Result : uint8 {
	Success = 0,
	MalformedRequest = 1,
	UnsupportedVersion = 2,
	InvalidSlot = 3,
	InvalidSession = 4,
	StaleRequest = 5,
	CursorMismatch = 6,
	SlotOccupied = 7,
	SlotEmpty = 8,
	CursorNotEmpty = 9,
	ItemNotAllowed = 10,
	DatabaseError = 11,
	Busy = 12
};

#pragma pack(push, 1)

struct SlotRecord {
	uint8 slot_index;
	uint8 occupied;
	uint16 reserved;
	uint32 revision;
	uint32 item_id;
	uint32 icon;
	uint32 charges;
	uint64 guid;
	char item_name[64];
};

struct List {
	uint16 protocol_version;
	uint16 slot_count;
	uint32 character_id;
	uint64 session_nonce;
	SlotRecord slots[SlotCount];
};

struct ActionRequest {
	uint16 protocol_version;
	uint8 action;
	uint8 slot_index;
	uint32 source_slot;
	uint32 expected_item_id;
	uint32 expected_revision;
	uint64 session_nonce;
	uint64 request_id;
	uint64 expected_guid;
};

struct ActionResult {
	uint16 protocol_version;
	uint8 result;
	uint8 slot_index;
	uint32 character_id;
	uint32 revision;
	uint64 request_id;
	SlotRecord slots[SlotCount];
};

#pragma pack(pop)

static_assert(sizeof(SlotRecord) == 92, "NMS Anything slot ABI changed");
static_assert(sizeof(List) == 200, "NMS Anything list ABI changed");
static_assert(sizeof(ActionRequest) == 40, "NMS Anything action ABI changed");
static_assert(sizeof(ActionResult) == 204, "NMS Anything result ABI changed");

} // namespace NMSAnything

#endif
