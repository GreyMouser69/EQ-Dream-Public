# Tavern Duels on your own server

## Linux service (Node.js 24)

The prebuilt `service/` directory runs with Node 24 and its built-in SQLite module; no npm install is needed to run it. Copy it to `/opt/eqdream-tavern/service`. Copy `config.example.json` to `/etc/eqdream-tavern.json`, and set `origin` to YOUR HTTPS origin (no /tavern suffix). Keep `secure: true`. The service listens on loopback port 17867.

Create a dedicated `eqdream-tavern` service user/group. Create `/var/lib/eqdream-tavern/data` owned by that user, mode 0700. Create `/var/lib/eqdream-tavern/tickets` owned by the game zone-service user with group `eqdream-tavern`, mode 2770. The zone-service user must be able to write tickets, and the Tavern service user must read and remove them. Never make these directories public web roots.

Install `eqdream-tavern.service` into systemd, adjusting the Node executable path if needed. Enable/start it after configuration. This starts the card service only; it does not restart EQ.

In your HTTPS nginx server block, add:

```nginx
location ^~ /tavern/ {
    access_log off;
    proxy_set_header Host $host;
    proxy_set_header X-Forwarded-Proto $scheme;
    proxy_pass http://127.0.0.1:17867/;
}
```

Retain your TLS configuration. Check nginx configuration before reloading. The trailing slash on proxy_pass is required. Test `/tavern/health` through YOUR HTTPS hostname.

## EQ quest hook

Copy `../quest-plugins/tavern_access.pl` into your installed quest plugins directory. Merge this branch at the START of the existing global player `EVENT_SAY` (do not add a second EVENT_SAY or replace other handlers):

```perl
if ($text eq '!tavern') {
    plugin::TavernAccess($client);
    return;
}
```

Reload global quests using your operator tooling. The player sends `!tavern`; the server sends a private one-use ticket based on the authenticated character ID. Never substitute a browser-supplied character identity.

## Windows player files

Requires MacroQuest RoF2 with MQ2Lua, windowed EQ, and Microsoft Edge WebView2 Runtime. Merge `client/lua/` into MacroQuest's `lua/`; merge `client/TavernDuels/` into MacroQuest's `TavernDuels/`. Create MacroQuest `config/tavern-live/`. Edit `TavernDuels/server-url.txt` to YOUR HTTPS origin, matching the server config. Do not use the example hostname.

Start MQ and EQ, log in, then run `/lua run eqdream_tavern_live` followed by `/tavern`. For automatic registration append `/lua run eqdream_tavern_live quiet` to MacroQuest `config/ingame.cfg`, preserving existing lines. `/tavern close` closes the window.

The supplied helper was rebuilt with configurable origin and without the original NMS-Local folder restriction. Rebuild it with `Build-Window.ps1`; WebView2 library notices are included. It restricts navigation to the configured origin.

## Source and testing

`source/` contains the UI, rules, and service sources. Install dependencies from its pnpm lockfile, then run `node node_modules/vite/bin/vite.js build --config live/vite.config.mjs` followed by `node live/build-service.mjs`. This creates `dist-live/`. Use `node live/test.mjs` and `node live/rewards-test.mjs` after building. The ready-to-run equivalent is `service/`; the bundled tests under `tests/` run against it.

Optional first-kill rewards: `EQDreamTavernRewards.pm` and its dependency `EQDreamRelease.pm` are included as references. To enable, configure a `rewards` path in the service config, create its `pending` and `receipts` directories with shared game/service group permissions (2770), install the modules, and call `EQDreamTavernRewards::killed_merit($client,$npc)` from your existing global NPC `EVENT_KILLED_MERIT` after existing credit logic. Add `use lib 'quests/plugins'; use EQDreamTavernRewards;` at the top. Review the classifier against your NPC configuration. This optional hook is NOT automatically installed.

Back up cards.sqlite using SQLite backup tooling, and the rewards outbox/receipts if enabled. Never publish those files or launch tickets. Diagnose unknown `/tavern` via Lua loading; a sign-in timeout via the quest hook/ticket permissions; a window connection failure via origin/nginx/WebView2.
