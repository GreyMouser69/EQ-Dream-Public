# 8. Classic through Luclin chase loot

The complete custom chase-loot handoff is under `chase-loot/`.

| Era | Server item IDs | Custom client package |
| --- | --- | --- |
| Classic — Flame of the First Age | `900100–900106` | `eqdream_chase_weapons.eqg` |
| Kunark — Dawnfang | `900120–900126` | `eqdream_dying_sun.eqg` |
| Velious — Winter’s Requiem | `900600–900605` | `eqdream_velious_20k.eqg` |
| Luclin — Moonsong/Eclipse | `900700–900712` | Luclin weapon, helm, robe, particle, and Moonsong EQG packages |

## Install order

1. Import the appropriate SQL from `chase-loot/server-data/` after the base database.
2. Install the matching custom client patch assets and append their resource mappings as documented in `chase-loot/client-patches/README.md`.
3. Install/reload the associated quest/plugin logic where applicable, then rebuild server shared memory/cache.
4. Restart the world and zone workers, fully restart the client, and verify an item from each era in game.

Do not mix a new SQL item set with older model archives or resource mappings. Preserve the SHA-256 checksums in `SHA256SUMS.txt` when moving patches between hosts.
