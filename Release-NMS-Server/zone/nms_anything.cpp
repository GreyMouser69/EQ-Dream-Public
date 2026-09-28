#include "nms_anything.h"

#include "client.h"
#include "zonedb.h"

#include "../common/inventory_slot.h"
#include "../common/repositories/character_anything_slots_repository.h"

#include <algorithm>
#include <array>
#include <limits>
#include <memory>
#include <random>
#include <unordered_map>

namespace {

struct SessionState {
	uint64 nonce = 0;
	uint64 last_request_id = 0;
};

std::unordered_map<uint32, SessionState> sessions;

uint64 NewNonce()
{
	static std::mt19937_64 generator(std::random_device{}());
	uint64 nonce = generator();
	return nonce ? nonce : 1;
}

std::array<NMSAnything::SlotRecord, NMSAnything::SlotCount> BuildRecords(Client *client)
{
	std::array<NMSAnything::SlotRecord, NMSAnything::SlotCount> records{};
	std::array<uint32, NMSAnything::SlotCount> revisions{};
	for (const auto &row : CharacterAnythingSlotsRepository::GetForCharacter(database, client->CharacterID())) {
		if (row.slot_index < revisions.size()) {
			revisions[row.slot_index] = row.revision;
		}
	}

	for (uint8 slot = 0; slot < records.size(); ++slot) {
		auto &record = records[slot];
		record.slot_index = slot;
		record.revision = revisions[slot];
		const auto *inst = client->GetAnythingItem(slot);
		if (!inst || !inst->GetItem()) {
			continue;
		}
		record.occupied = 1;
		record.item_id = inst->GetID();
		record.icon = inst->GetItem()->Icon;
		record.charges = inst->GetCharges() < 0 ? 0xFFFFFFFFu : static_cast<uint32>(inst->GetCharges());
		record.guid = inst->GetSerialNumber();
		strn0cpy(record.item_name, inst->GetItem()->Name, sizeof(record.item_name));
	}
	return records;
}

void SendList(Client *client, uint64 nonce)
{
	auto packet = new EQApplicationPacket(OP_NMSAnythingList, sizeof(NMSAnything::List));
	auto list = reinterpret_cast<NMSAnything::List *>(packet->pBuffer);
	list->protocol_version = NMSAnything::ProtocolVersion;
	list->slot_count = NMSAnything::SlotCount;
	list->character_id = client->CharacterID();
	list->session_nonce = nonce;
	const auto records = BuildRecords(client);
	std::copy(records.begin(), records.end(), std::begin(list->slots));
	client->QueuePacket(packet);
	safe_delete(packet);
}

void SendResult(Client *client, const NMSAnything::ActionRequest &request, NMSAnything::Result result)
{
	auto packet = new EQApplicationPacket(OP_NMSAnythingResult, sizeof(NMSAnything::ActionResult));
	auto response = reinterpret_cast<NMSAnything::ActionResult *>(packet->pBuffer);
	response->protocol_version = NMSAnything::ProtocolVersion;
	response->result = static_cast<uint8>(result);
	response->slot_index = request.slot_index;
	response->character_id = client->CharacterID();
	response->request_id = request.request_id;
	const auto records = BuildRecords(client);
	std::copy(records.begin(), records.end(), std::begin(response->slots));
	response->revision = request.slot_index < records.size() ? records[request.slot_index].revision : 0;
	client->QueuePacket(packet);
	safe_delete(packet);
}

CharacterAnythingSlotsRepository::Slot ToStoredSlot(
	Client *client, uint8 slot_index, const EQ::ItemInstance *inst, uint32 revision
)
{
	CharacterAnythingSlotsRepository::Slot row;
	row.character_id = client->CharacterID();
	row.slot_index = slot_index;
	row.item_id = inst->GetID();
	row.charges = inst->GetCharges() < 0 ? std::numeric_limits<uint16>::max() : static_cast<uint16>(inst->GetCharges());
	row.color = inst->GetColor();
	for (uint8 i = EQ::invaug::SOCKET_BEGIN; i <= EQ::invaug::SOCKET_END; ++i) {
		const auto *augment = inst->GetAugment(i);
		row.augments[i] = augment ? augment->GetID() : 0;
	}
	row.instnodrop = inst->IsAttuned() ? 1 : 0;
	row.custom_data = inst->GetCustomDataString();
	row.ornament_icon = inst->GetOrnamentationIcon();
	row.ornament_idfile = inst->GetOrnamentationIDFile();
	row.ornament_hero_model = inst->GetOrnamentHeroModel();
	if (inst->GetSerialNumber()) {
		row.guid = inst->GetSerialNumber();
	}
	row.revision = revision;
	return row;
}

bool Matches(const CharacterAnythingSlotsRepository::Slot &row, uint32 item_id, uint64 guid)
{
	return row.item_id == item_id && row.guid.value_or(0) == guid;
}

} // namespace

namespace NMSAnything {

void HandleAction(Client *client, const ActionRequest &request)
{
	if (!client) {
		return;
	}

	auto &session = sessions[client->CharacterID()];
	const auto action = static_cast<Action>(request.action);
	if (action == Action::Refresh) {
		session.nonce = NewNonce();
		session.last_request_id = 0;
		SendList(client, session.nonce);
		return;
	}

	if (request.session_nonce == 0 || request.session_nonce != session.nonce) {
		SendResult(client, request, Result::InvalidSession);
		return;
	}
	if (request.request_id == 0 || request.request_id <= session.last_request_id) {
		SendResult(client, request, Result::StaleRequest);
		return;
	}
	session.last_request_id = request.request_id;

	if (action != Action::EquipCursor && action != Action::WithdrawCursor) {
		SendResult(client, request, Result::MalformedRequest);
		return;
	}
	if (request.slot_index >= SlotCount) {
		SendResult(client, request, Result::MalformedRequest);
		return;
	}

	if (action == Action::EquipCursor) {
		if (request.source_slot != static_cast<uint32>(EQ::invslot::slotCursor) || client->GetAnythingItem(request.slot_index)) {
			SendResult(client, request, client->GetAnythingItem(request.slot_index) ? Result::SlotOccupied : Result::CursorMismatch);
			return;
		}

		const auto *cursor = client->GetInv().GetItem(EQ::invslot::slotCursor);
		if (!cursor || !cursor->GetItem() || cursor->IsClassBag() || request.expected_revision != 0) {
			SendResult(client, request, cursor && cursor->IsClassBag() ? Result::ItemNotAllowed : Result::CursorMismatch);
			return;
		}
		const auto cursor_item_id = cursor->GetID();

		database.TransactionBegin();
		const auto destination = CharacterAnythingSlotsRepository::LockSlot(database, client->CharacterID(), request.slot_index);
		const auto source = CharacterAnythingSlotsRepository::LockInventorySlot(database, client->CharacterID(), request.source_slot);
		if (destination || !source || source->item_id != cursor_item_id) {
			database.TransactionRollback();
			SendResult(client, request, destination ? Result::SlotOccupied : Result::CursorMismatch);
			return;
		}

		// The live ItemInstance serial is not guaranteed to equal the persisted
		// inventory GUID after zoning or a server restart.  The locked inventory
		// row is authoritative for the durable move; the live cursor item ID was
		// already checked above to prevent moving a different item.
		auto stored = *source;
		stored.character_id = client->CharacterID();
		stored.slot_index = request.slot_index;
		stored.revision = 1;
		const bool saved = CharacterAnythingSlotsRepository::Upsert(database, stored) &&
			CharacterAnythingSlotsRepository::DeleteInventorySlot(
				database, client->CharacterID(), request.source_slot, cursor_item_id, source->guid
			) && CharacterAnythingSlotsRepository::RecordAudit(
				database, client->CharacterID(), request.slot_index, request.request_id,
				"equip", stored.item_id, stored.guid, "success"
			);
		if (!saved || !database.TransactionCommit().Success()) {
			database.TransactionRollback();
			SendResult(client, request, Result::DatabaseError);
			return;
		}

		client->DeleteItemInInventory(EQ::invslot::slotCursor, 0, true, false);
		client->LoadAnythingSlots();
		client->CalcBonuses();
		SendResult(client, request, Result::Success);
		return;
	}

	if (client->GetInv().GetItem(EQ::invslot::slotCursor)) {
		SendResult(client, request, Result::CursorNotEmpty);
		return;
	}
	const auto *anything_item = client->GetAnythingItem(request.slot_index);
	if (!anything_item || anything_item->GetID() != request.expected_item_id ||
		anything_item->GetSerialNumber() != request.expected_guid) {
		SendResult(client, request, Result::SlotEmpty);
		return;
	}
	auto withdrawn = std::unique_ptr<EQ::ItemInstance>(anything_item->Clone());

	database.TransactionBegin();
	const auto source = CharacterAnythingSlotsRepository::LockSlot(database, client->CharacterID(), request.slot_index);
	const auto cursor_row = CharacterAnythingSlotsRepository::LockInventorySlot(
		database, client->CharacterID(), EQ::invslot::slotCursor
	);
	if (!source || cursor_row || source->revision != request.expected_revision ||
		!Matches(*source, request.expected_item_id, request.expected_guid)) {
		database.TransactionRollback();
		SendResult(client, request, cursor_row ? Result::CursorNotEmpty : Result::StaleRequest);
		return;
	}

	const bool saved = CharacterAnythingSlotsRepository::InsertInventorySlot(
		database, client->CharacterID(), EQ::invslot::slotCursor, *source
	) && CharacterAnythingSlotsRepository::Delete(
		database, client->CharacterID(), request.slot_index, request.expected_revision
	) && CharacterAnythingSlotsRepository::RecordAudit(
		database, client->CharacterID(), request.slot_index, request.request_id,
		"withdraw", source->item_id, source->guid, "success"
	);
	if (!saved || !database.TransactionCommit().Success()) {
		database.TransactionRollback();
		SendResult(client, request, Result::DatabaseError);
		return;
	}

	client->LoadAnythingSlots();
	if (!client->PutItemInInventory(EQ::invslot::slotCursor, *withdrawn, true)) {
		LogError("NMS Anything: committed withdrawal but failed to refresh cursor for [{}]", client->GetCleanName());
	}
	client->CalcBonuses();
	SendResult(client, request, Result::Success);
}

} // namespace NMSAnything
