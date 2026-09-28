# Chase-loot server data

These SQL files are a **test-database snapshot** of the four chase-loot sets, exported without account, character, inventory, guild, mail, corpse, or other player data.

Apply them in expansion order after importing the base database:

```bash
mysql -u eqdream -p eqdream < classic-items.sql
mysql -u eqdream -p eqdream < kunark-items.sql
mysql -u eqdream -p eqdream < velious-items.sql
mysql -u eqdream -p eqdream < luclin-items.sql
mysql -u eqdream -p eqdream < luclin-spells.sql
```

The files use `REPLACE INTO`; on an existing server this replaces only the listed item/spell IDs. Back up first, rebuild server shared memory/cache, and restart world/zone processes after applying.

Set membership: Classic `900100–900106`; Kunark `900120–900126`; Velious `900600–900605`; Luclin `900700–900712`; Luclin spell rows `42900–42917`.
