-- EQ Dream: add three 9-point ranks to Enchanter and Druid Dire Charm.
-- Rank caps are enforced server-side at 46, 56, 66, and 76.

DELETE FROM aa_ranks WHERE id BETWEEN 50000 AND 50005;

UPDATE aa_ranks SET next_id = 19000 WHERE id = 145;
INSERT INTO aa_ranks
    (id, upper_hotkey_sid, lower_hotkey_sid, title_sid, desc_sid, cost, level_req, spell, spell_type, recast_time, expansion, prev_id, next_id)
VALUES
    (19000, 145, 145, 145, 145, 9, 1, 2761, 2, 900, 3, 145, 19001),
    (19001, 145, 145, 145, 145, 9, 1, 2761, 2, 900, 3, 19000, 19002),
    (19002, 145, 145, 145, 145, 9, 1, 2761, 2, 900, 3, 19001, -1)
ON DUPLICATE KEY UPDATE
    upper_hotkey_sid = VALUES(upper_hotkey_sid),
    lower_hotkey_sid = VALUES(lower_hotkey_sid),
    title_sid = VALUES(title_sid),
    desc_sid = VALUES(desc_sid),
    cost = VALUES(cost),
    level_req = VALUES(level_req),
    spell = VALUES(spell),
    spell_type = VALUES(spell_type),
    recast_time = VALUES(recast_time),
    expansion = VALUES(expansion),
    prev_id = VALUES(prev_id),
    next_id = VALUES(next_id);

UPDATE aa_ranks SET next_id = 19003 WHERE id = 960;
INSERT INTO aa_ranks
    (id, upper_hotkey_sid, lower_hotkey_sid, title_sid, desc_sid, cost, level_req, spell, spell_type, recast_time, expansion, prev_id, next_id)
VALUES
    (19003, 960, 960, 960, 960, 9, 1, 2760, 7, 900, 3, 960, 19004),
    (19004, 960, 960, 960, 960, 9, 1, 2760, 7, 900, 3, 19003, 19005),
    (19005, 960, 960, 960, 960, 9, 1, 2760, 7, 900, 3, 19004, -1)
ON DUPLICATE KEY UPDATE
    upper_hotkey_sid = VALUES(upper_hotkey_sid),
    lower_hotkey_sid = VALUES(lower_hotkey_sid),
    title_sid = VALUES(title_sid),
    desc_sid = VALUES(desc_sid),
    cost = VALUES(cost),
    level_req = VALUES(level_req),
    spell = VALUES(spell),
    spell_type = VALUES(spell_type),
    recast_time = VALUES(recast_time),
    expansion = VALUES(expansion),
    prev_id = VALUES(prev_id),
    next_id = VALUES(next_id);
