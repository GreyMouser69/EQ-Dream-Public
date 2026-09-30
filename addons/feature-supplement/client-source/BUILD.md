# Custom RoF2 client extension

The `client/` directory is the maintained source snapshot for the custom NMS dinput8 extension, including Spell Shards and item presentation. It is version-specific to the compatible 32-bit RoF2 client. Source files preserve upstream license headers.

Install Visual Studio C++ desktop build tools with the v142 toolset and a Windows 10 SDK (project default 10.0.18362.0; override WindowsTargetPlatformVersion if using a compatible installed SDK). Build `client/eqgame_dll.vcxproj` in Release/Win32, NOT dinput8.vcxproj (that is the proxy-only project).

```powershell
msbuild client/eqgame_dll.vcxproj /p:Configuration=Release /p:Platform=Win32
```

The relative Detours and DirectX dependency folders are supplied alongside client/. Do not flatten them. Use the resulting custom dinput8.dll with the bundled Forge XML/artwork and matching server opcodes. The prebuilt client-files/dinput8.dll is the maintained local client artifact; a fresh full native rebuild and in-game compatibility check on the recipient's setup remain required before distributing modified builds.
