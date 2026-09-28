# 4. Content installation

The public repository already contains the EQ Dream quest and plugin trees:

- `Release-NMS-Quests/`
- `Release-NMS-Plugins/`

Configure the server's quest path to use these directories, or copy them into the server's configured quest root. A typical layout is:

```bash
mkdir -p /home/eqdream/server/quests
cp -a /home/eqdream/NMS-Release/Release-NMS-Quests/. /home/eqdream/server/quests/
mkdir -p /home/eqdream/server/quests/plugins
cp -a /home/eqdream/NMS-Release/Release-NMS-Plugins/. /home/eqdream/server/quests/plugins/
```

Keep the quest and plugin trees together at the same repository revision as the database and source. They are code: review changes before deploying them, and restart/reload only in a maintenance window when a quest change requires it.