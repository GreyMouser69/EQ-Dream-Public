# 5. Client setup

EQ Dream does not provide or distribute an EverQuest executable, stock S3D/EQG assets, or account access. Each operator must use a lawfully obtained compatible RoF2 client. Custom chase-loot packages are under `chase-loot/client-patches/`.

For `/spellshards` (plural), `/tavern`, and Mythic stats, install the [feature supplement](../addons/feature-supplement/README.md), also available as a [ZIP with instructions](../downloads/EQ-Dream-Feature-Supplement-2026-09-29.zip). Spell Shards needs the custom client DLL/UI and matching server source/opcodes/currency. Tavern needs MacroQuest Lua, the window helper, the quest authentication hook, and your own HTTPS card service; configure your own address. A server-only checkout does not provide these client commands.

On a test client, configure the server endpoint in that client's connection configuration (for example, `eqhost.txt`) to your own hostname/IP and login port. Keep the client separate from the server repository. Test the connection locally first, then test from an external network after firewall rules are in place.

Never copy a production client configuration, launcher credentials, or live asset package into this public repository.
