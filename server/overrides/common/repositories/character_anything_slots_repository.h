#ifndef EQEMU_CHARACTER_ANYTHING_SLOTS_REPOSITORY_H
#define EQEMU_CHARACTER_ANYTHING_SLOTS_REPOSITORY_H

#include "../database.h"
#include "../strings.h"

#include <optional>
#include <string>
#include <vector>

class CharacterAnythingSlotsRepository {
public:
	struct Slot {
		uint32 character_id = 0;
		uint8 slot_index = 0;
		uint32 item_id = 0;
		uint16 charges = 0;
		uint32 color = 0;
		uint32 augments[6]{};
		uint8 instnodrop = 0;
		std::string custom_data;
		uint32 ornament_icon = 0;
		uint32 ornament_idfile = 0;
		int32 ornament_hero_model = 0;
		std::optional<uint64> guid;
		uint32 revision = 1;
	};

	static std::vector<Slot> GetForCharacter(Database &db, uint32 character_id)
	{
		return Select(db, fmt::format("`character_id` = {} ORDER BY `slot_index`", character_id));
	}

	// Caller must have started a transaction. The lock serializes competing
	// equip/withdraw requests for one custom slot.
	static std::optional<Slot> LockSlot(Database &db, uint32 character_id, uint8 slot_index)
	{
		auto rows = Select(
			db,
			fmt::format("`character_id` = {} AND `slot_index` = {} FOR UPDATE", character_id, slot_index)
		);
		return rows.empty() ? std::nullopt : std::optional<Slot>(rows.front());
	}

	// Reads and locks a native inventory row using the same persisted item
	// representation as an Anything slot. The returned slot_index is not useful
	// for native slots; callers already know source_slot.
	static std::optional<Slot> LockInventorySlot(Database &db, uint32 character_id, uint32 source_slot)
	{
		auto result = db.QueryDatabase(fmt::format(
			"SELECT `character_id`,0,`item_id`,`charges`,`color`,`augment_one`,`augment_two`,`augment_three`,"
			"`augment_four`,`augment_five`,`augment_six`,`instnodrop`,`custom_data`,`ornament_icon`,"
			"`ornament_idfile`,`ornament_hero_model`,NULLIF(`guid`,0),1 FROM `inventory` "
			"WHERE `character_id` = {} AND `slot_id` = {} FOR UPDATE",
			character_id, source_slot
		));
		if (!result.Success() || result.RowCount() != 1) {
			return std::nullopt;
		}
		return FromRow(result.begin());
	}

	static bool DeleteInventorySlot(
		Database &db, uint32 character_id, uint32 source_slot, uint32 expected_item_id,
		const std::optional<uint64> &expected_guid
	)
	{
		const auto guid_predicate = expected_guid
			? fmt::format("`guid` = {}", *expected_guid)
			: "(`guid` = 0 OR `guid` IS NULL)";
		auto result = db.QueryDatabase(fmt::format(
			"DELETE FROM `inventory` WHERE `character_id` = {} AND `slot_id` = {} AND `item_id` = {} AND {}",
			character_id, source_slot, expected_item_id, guid_predicate
		));
		return result.Success() && result.RowsAffected() == 1;
	}

	// Use only after locking and proving the native destination slot is empty.
	// Plain INSERT is intentional: a collision must fail instead of overwriting
	// an item that appeared during the request.
	static bool InsertInventorySlot(Database &db, uint32 character_id, uint32 destination_slot, const Slot &s)
	{
		const auto guid = s.guid ? std::to_string(*s.guid) : "0";
		auto result = db.QueryDatabase(fmt::format(
			"INSERT INTO `inventory` (`character_id`,`slot_id`,`item_id`,`charges`,`color`,`augment_one`,"
			"`augment_two`,`augment_three`,`augment_four`,`augment_five`,`augment_six`,`instnodrop`,"
			"`custom_data`,`ornament_icon`,`ornament_idfile`,`ornament_hero_model`,`guid`) VALUES "
			"({},{},{},{},{},{},{},{},{},{},{},{},'{}',{},{},{},{})",
			character_id, destination_slot, s.item_id, s.charges, s.color, s.augments[0], s.augments[1],
			s.augments[2], s.augments[3], s.augments[4], s.augments[5], s.instnodrop,
			Strings::Escape(s.custom_data), s.ornament_icon, s.ornament_idfile, s.ornament_hero_model, guid
		));
		return result.Success() && result.RowsAffected() == 1;
	}

	static bool Upsert(Database &db, const Slot &s)
	{
		const auto guid = s.guid ? std::to_string(*s.guid) : "NULL";
		auto result = db.QueryDatabase(fmt::format(
			"INSERT INTO `character_anything_slots` "
			"(`character_id`,`slot_index`,`item_id`,`charges`,`color`,`augment_one`,`augment_two`,`augment_three`,"
			"`augment_four`,`augment_five`,`augment_six`,`instnodrop`,`custom_data`,`ornament_icon`,"
			"`ornament_idfile`,`ornament_hero_model`,`guid`,`revision`) VALUES "
			"({},{},{},{},{},{},{},{},{},{},{},{},'{}',{},{},{},{},{}) "
			"ON DUPLICATE KEY UPDATE `item_id`=VALUES(`item_id`),`charges`=VALUES(`charges`),"
			"`color`=VALUES(`color`),`augment_one`=VALUES(`augment_one`),`augment_two`=VALUES(`augment_two`),"
			"`augment_three`=VALUES(`augment_three`),`augment_four`=VALUES(`augment_four`),"
			"`augment_five`=VALUES(`augment_five`),`augment_six`=VALUES(`augment_six`),"
			"`instnodrop`=VALUES(`instnodrop`),`custom_data`=VALUES(`custom_data`),"
			"`ornament_icon`=VALUES(`ornament_icon`),`ornament_idfile`=VALUES(`ornament_idfile`),"
			"`ornament_hero_model`=VALUES(`ornament_hero_model`),`guid`=VALUES(`guid`),"
			"`revision`=`revision`+1",
			s.character_id, s.slot_index, s.item_id, s.charges, s.color,
			s.augments[0], s.augments[1], s.augments[2], s.augments[3], s.augments[4], s.augments[5],
			s.instnodrop, Strings::Escape(s.custom_data), s.ornament_icon, s.ornament_idfile,
			s.ornament_hero_model, guid, s.revision
		));
		return result.Success();
	}

	static bool Delete(Database &db, uint32 character_id, uint8 slot_index, uint32 expected_revision)
	{
		auto result = db.QueryDatabase(fmt::format(
			"DELETE FROM `character_anything_slots` WHERE `character_id` = {} AND `slot_index` = {} AND `revision` = {}",
			character_id, slot_index, expected_revision
		));
		return result.Success() && result.RowsAffected() == 1;
	}

	static bool RecordAudit(
		Database &db, uint32 character_id, uint8 slot_index, uint64 request_id,
		const std::string &action, uint32 item_id, const std::optional<uint64> &guid,
		const std::string &result_text
	)
	{
		const auto guid_sql = guid ? std::to_string(*guid) : "NULL";
		auto result = db.QueryDatabase(fmt::format(
			"INSERT INTO `character_anything_slot_audit` "
			"(`character_id`,`slot_index`,`request_id`,`action`,`item_id`,`guid`,`result`) "
			"VALUES ({},{},{},'{}',{},{},'{}')",
			character_id, slot_index, request_id, Strings::Escape(action), item_id, guid_sql,
			Strings::Escape(result_text)
		));
		return result.Success();
	}

private:
	template <typename Row>
	static Slot FromRow(Row &row)
	{
		Slot s;
		s.character_id = Strings::ToUnsignedInt(row[0]);
		s.slot_index = static_cast<uint8>(Strings::ToUnsignedInt(row[1]));
		s.item_id = Strings::ToUnsignedInt(row[2]);
		s.charges = static_cast<uint16>(Strings::ToUnsignedInt(row[3]));
		s.color = Strings::ToUnsignedInt(row[4]);
		for (int i = 0; i < 6; ++i) {
			s.augments[i] = Strings::ToUnsignedInt(row[5 + i]);
		}
		s.instnodrop = static_cast<uint8>(Strings::ToUnsignedInt(row[11]));
		s.custom_data = row[12] ? row[12] : "";
		s.ornament_icon = Strings::ToUnsignedInt(row[13]);
		s.ornament_idfile = Strings::ToUnsignedInt(row[14]);
		s.ornament_hero_model = Strings::ToInt(row[15]);
		if (row[16]) {
			s.guid = Strings::ToUnsignedBigInt(row[16]);
		}
		s.revision = Strings::ToUnsignedInt(row[17]);
		return s;
	}

	static std::vector<Slot> Select(Database &db, const std::string &filter)
	{
		std::vector<Slot> slots;
		auto result = db.QueryDatabase(fmt::format(
			"SELECT `character_id`,`slot_index`,`item_id`,`charges`,`color`,`augment_one`,`augment_two`,"
			"`augment_three`,`augment_four`,`augment_five`,`augment_six`,`instnodrop`,`custom_data`,"
			"`ornament_icon`,`ornament_idfile`,`ornament_hero_model`,`guid`,`revision` "
			"FROM `character_anything_slots` WHERE {}", filter
		));
		if (!result.Success()) {
			return slots;
		}
		for (auto row = result.begin(); row != result.end(); ++row) {
			slots.push_back(FromRow(row));
		}
		return slots;
	}
};

#endif
