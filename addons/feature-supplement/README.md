# EQ Dream feature supplement

For the initial `ssinjinx/EQ-Dream-Public` release (d086d756ef18e58388c7a2f490b6ed529f3853f6). Adds the missing Spell Shards components, portable Tavern Duels service/client, and current Mythic stat definitions. No accounts, characters, passwords, tickets, collections, or EQ executable are included.

## Installation order

Use a private test copy first. Back up your database, binaries, opcode file, quests and client. Do not reload the public base database over an existing server. Arrange downtime on YOUR server before replacing binaries/caches.

1. Review `server-overlay-files.json`. Copy `server-overlay/` into your `Release-NMS-Server/` source, preserving paths, then rebuild/install following the public repository build guide. This is a matched 65-file snapshot, including shared dependencies and other maintained EQ Dream fixes, not an isolated minimal patch. Diff your local modifications first.
2. Merge `spell-shard-opcodes.txt` into your installed `patch_RoF2.conf`, replacing those two entries only. Client and server must agree on 0x140f and 0x1410.
3. Import the content below into YOUR database; replace `eqdream` with its name and supply your own authentication. Review ID conflicts first. SQL replaces content definitions, never inventory or characters.

```sh
gzip -dc database/spell-shard-items.sql.gz | mariadb eqdream
mariadb eqdream < database/spell-shard-currency.sql
gzip -dc database/mythic-items.sql.gz | mariadb eqdream
```

4. Add bundle item 990102 to a free slot on YOUR Echo of Memory merchant, with `alt_currency_cost=1` and `faction_required=-1100`. Currency 6 is Echo of Memory; currency 7 is Spell Shards (item 990101). `database/merchant-reference.tsv` shows the original merchant mapping. The server handler converts each bundle purchase to five shards. This snapshot does not include a later 100-shard purchase option.
5. With game services stopped, build a new shared cache from the updated database in the installed server directory: `./shared_memory -hotfix=features_20260929_`. After both prefixed items/spells files exist, set `variables.hotfix_name` to `features_20260929_` and start services. Verify each worker maps that prefix in `/proc/PID/maps`. Restarting with an old cache does not load new item definitions.
6. Close EQ and run `Install-SpellShards.ps1 -ClientPath 'C:\Games\YourRoF2Client'` from PowerShell. It backs up and installs the custom DLL and Forge UI. Requires the compatible 32-bit RoF2 executable used by NMS. Custom UIs need the same Forge include; test the default UI first. Corresponding maintained client source is included; see `client-source/BUILD.md`.
7. Install Tavern using `tavern/INSTALL.md` and configure YOUR HTTPS address in both the service configuration and client `server-url.txt`.

## Commands and Mythic behavior

**`/spellshards` is plural.** It is registered by the custom client DLL. Unknown command means the DLL is absent/not loaded in the running client. A window without synchronization suggests missing server opcode handlers or mismatched binaries. Progress uses existing data buckets and alternate-currency tables.

**`/tavern`** is registered by MacroQuest Lua. Start MQ/MQ2Lua, log in, then `/lua run eqdream_tavern_live`. Server-only installation cannot register it.

The Mythic archive contains 63,775 content definitions, including weapons, in the 4-million ID tier. It restores missing definitions and updates stats; it does not grant items or convert existing inventory to Mythic. Reference Ragebringer (Mythic): damage 60, delay 25, HP 400. The public tier/upgrade logic remains in use; `quest-plugins/NMS_item_utils.pl` is included for comparison. Epic quest reward selections and chase weapon form switching are not changed by this import. Existing locally customized Mythic rows are overwritten: review first.

## Acceptance checks

Log into YOUR server with the patched client. Open `/spellshards`, bind a memorized spell, buy a rank, relog, and check persistence and payment. Inspect a freshly summoned Mythic item against its database row and test your usual upgrade path. Open Tavern with two test characters, verify separate collections, play a shared duel, close/reopen, and verify persistence. Check UIErrors.txt and server/service logs. Automated package checks do not establish in-game acceptance on another operator's installation.

## Rollback

Close EQ and restore the installer backup. Restore pre-install binaries, quests, opcode file, content backup and cache prefix before restarting your services. Keep player data; never restore a fresh base database over it. Back up Tavern cards.sqlite using SQLite backup tooling before a service rollback.

Third-party code retains its own license notices in source headers and client/WebView2 notice files. No stock EverQuest archives are included.
