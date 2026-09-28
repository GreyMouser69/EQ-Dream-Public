# 2. Build the server

From the repository root, first copy the EQ Dream public overrides into the server source tree. This step is explicit so a future maintainer can review every custom server-side file.

```bash
cd /home/eqdream/NMS-Release
cp server/overrides/common/nms_anything_structs.h Release-NMS-Server/common/
cp server/overrides/common/repositories/character_anything_slots_repository.h Release-NMS-Server/common/repositories/
cp server/overrides/zone/nms_anything.cpp Release-NMS-Server/zone/
cp server/overrides/zone/nms_anything.h Release-NMS-Server/zone/
```

Then build:

```bash
cd Release-NMS-Server
make
```

Binaries are produced in `build/bin/`. On Windows, run `build-windows.bat`, then build `Build/EQEmu.sln` as Release/x64; see `Release-NMS-Server/README.md`.

After any source, items, spells, or database-structure change, rebuild shared memory/cache using the server's normal utilities before restarting world and zone processes. Always verify startup logs before accepting players.