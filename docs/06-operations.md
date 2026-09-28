# 6. Operations

## First start

1. Create a real `eqemu_config.json` from `config/eqemu_config.json.example` and set unique database credentials, world key, server names, and addresses.
2. Keep the configuration readable only by the service account (`chmod 600`).
3. Build shared memory/cache using the server's normal tools.
4. Start the login service if self-hosting it, then world, then the launcher/zone workers.
5. Watch logs until world registers and every expected zone worker is healthy.
6. Log in with a new test account and verify character creation, zoning, and a quest interaction.

## Restart order

For planned maintenance: announce the maintenance window, prevent new logins, stop zone workers/launcher, stop world, make a database backup, apply the change, rebuild cache if needed, start world and workers, then verify health before reopening logins.

## Minimum monitoring

Monitor process state, world/zone logs, database disk space, backup completion, open ports, and player reports. Alert on repeated zone crashes, database errors, login failures, or cache/schema mismatches.

## Never publish

Do not commit `eqemu_config.json`, `login.json`, database dumps containing player data, logs, crash dumps, TLS/SSH keys, Discord/webhook URLs, or compiled client assets.