-- EQ Dream local-test Quick Buff AA conversion.
-- One free active rank; server behavior is implemented in zone/aa.cpp.

UPDATE `aa_ranks`
SET `cost` = 0,
    `upper_hotkey_sid` = 147,
    `lower_hotkey_sid` = 147,
    `spell` = 2749,
    `spell_type` = 2,
    `recast_time` = 0,
    `prev_id` = -1,
    `next_id` = -1
WHERE `id` = 147;

DELETE FROM `aa_rank_effects`
WHERE `rank_id` IN (147, 148, 149);

DELETE FROM `aa_rank_prereqs`
WHERE `rank_id` IN (147, 148, 149);

-- Keep the retired rows for safe character-data compatibility, but remove
-- them from the purchasable rank chain.
UPDATE `aa_ranks`
SET `prev_id` = -1, `next_id` = -1
WHERE `id` IN (148, 149);
