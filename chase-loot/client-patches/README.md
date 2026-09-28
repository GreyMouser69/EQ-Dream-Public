# Client patches — custom chase loot

These are the active custom EQ Dream chase-loot model/skin packages taken from the maintained test-client patch set. They cover the Classic (Flame of the First Age), Kunark (Dawnfang), Velious (Winter’s Requiem), and Luclin (Moonsong/Eclipse) chase-loot visuals.

## Install

1. Start with a lawfully obtained compatible RoF2 client.
2. Back up the client’s `Resources/GlobalLoad.txt` and `Resources/OnDemandResources.txt`.
3. Copy the `.eqg` files to the client root and `ActorEffects/moonsong_stars.dds` to `ActorEffects/`.
4. Append only the lines in `Resources/GlobalLoad.additions.txt` and `Resources/OnDemandResources.additions.txt` to the matching client resource files. Do not replace the whole resource files.
5. Fully exit and restart the client. Use the supplied SHA-256 manifest at the public-folder root to verify copied files.

These packages are custom patches for an existing client; they do not include a game client, stock assets, account data, or credentials. The separate SQL payload must be installed server-side for items to use these appearances.