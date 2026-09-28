-- EQ Dream: persistent storage for two server-authoritative Anything slots.
-- Slot indices are deliberately separate from the RoF2 inventory slot map.

CREATE TABLE IF NOT EXISTS `character_anything_slots` (
  `character_id` int unsigned NOT NULL,
  `slot_index` tinyint unsigned NOT NULL,
  `item_id` int unsigned NOT NULL,
  `charges` smallint unsigned NOT NULL DEFAULT 0,
  `color` int unsigned NOT NULL DEFAULT 0,
  `augment_one` int unsigned NOT NULL DEFAULT 0,
  `augment_two` int unsigned NOT NULL DEFAULT 0,
  `augment_three` int unsigned NOT NULL DEFAULT 0,
  `augment_four` int unsigned NOT NULL DEFAULT 0,
  `augment_five` int unsigned NOT NULL DEFAULT 0,
  `augment_six` int unsigned NOT NULL DEFAULT 0,
  `instnodrop` tinyint unsigned NOT NULL DEFAULT 0,
  `custom_data` text NOT NULL,
  `ornament_icon` int unsigned NOT NULL DEFAULT 0,
  `ornament_idfile` int unsigned NOT NULL DEFAULT 0,
  `ornament_hero_model` int NOT NULL DEFAULT 0,
  `guid` bigint unsigned DEFAULT NULL,
  `revision` int unsigned NOT NULL DEFAULT 1,
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`character_id`, `slot_index`),
  UNIQUE KEY `uq_character_anything_guid` (`guid`),
  CONSTRAINT `chk_character_anything_slot` CHECK (`slot_index` IN (0, 1))
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS `character_anything_slot_audit` (
  `id` bigint unsigned NOT NULL AUTO_INCREMENT,
  `character_id` int unsigned NOT NULL,
  `slot_index` tinyint unsigned NOT NULL,
  `request_id` bigint unsigned NOT NULL,
  `action` varchar(16) NOT NULL,
  `item_id` int unsigned NOT NULL DEFAULT 0,
  `guid` bigint unsigned DEFAULT NULL,
  `result` varchar(32) NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_anything_audit_character` (`character_id`, `created_at`),
  UNIQUE KEY `uq_anything_request` (`character_id`, `request_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
