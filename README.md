# EQ Dream public server handoff

This folder is the clean, public handoff for operating and extending an EQ Dream server. It documents the versioned source, quest content, plugins, database changes, build process, and daily operations without publishing player data, credentials, private infrastructure, or EverQuest client files.

## Start here

1. Read [Prerequisites](docs/01-prerequisites.md).
2. Build `Release-NMS-Server` using [the build guide](docs/02-server-build.md).
3. Create an empty MariaDB database and load the supplied base database as described in [Database setup](docs/03-database-setup.md).
4. Apply the EQ Dream migrations in `database/migrations/` and install the custom source overrides in `server/overrides/`.
5. Install the included quest/plugin trees already tracked at `Release-NMS-Quests/` and `Release-NMS-Plugins/` using [Content installation](docs/04-content-install.md).
6. Create your own configuration from `config/eqemu_config.json.example`; never commit the resulting real configuration.
7. Start and operate the services using [Operations](docs/06-operations.md).

## What is and is not included

Included in this repository: server source, clean base database archive, EQ Dream quests, plugins, public SQL migrations, and the public custom server-source files needed by this release.

Not included: EverQuest client executables, stock client assets, account/character/player data, production database dumps, passwords, API tokens, private keys, live hostnames/IPs, or private deployment tooling. The custom chase-loot patch packages, their resource mappings, and clean item/spell SQL snapshots are included under `chase-loot/`; use a lawfully obtained compatible RoF2 client; see [Client setup](docs/05-client-setup.md) and [Chase loot](docs/08-chase-loot.md).

## Repository layout

| Path | Purpose |
| --- | --- |
| `Release-NMS-Server/` | EQEmu-derived server source and base database archive |
| `Release-NMS-Quests/` | EQ Dream quest tree |
| `Release-NMS-Plugins/` | EQ Dream Perl plugin tree |
| `database/migrations/` | Public custom database migrations |
| `server/overrides/` | Reference copies of the custom server source already integrated under `Release-NMS-Server/` |
| `chase-loot/` | Classic, Kunark, Velious, and Luclin custom models/skins plus clean server-data snapshots |`n| `docs/` | Rebuild, configuration, client, and operations guides |

## License and trademarks

The server source remains under its upstream GPL-3.0 license; see `Release-NMS-Server/LICENSE`. EQEmu and its contributors retain their respective rights. EverQuest is a Daybreak Game Company trademark. EQ Dream is not affiliated with or endorsed by Daybreak. Do not distribute client files or other proprietary game assets through this repository.

## Security

All example values are deliberately non-working placeholders. Change every password/key, bind management services privately, use a firewall, and make tested backups before opening a server to players. Read [SECURITY.md](docs/SECURITY.md) before deployment.