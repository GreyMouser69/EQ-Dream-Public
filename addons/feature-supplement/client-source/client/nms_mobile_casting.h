#pragma once

// RoF2 test-only movement casting. No class spoofing or spell-data changes.
// A marker beside eqgame.exe is required, so this stays disabled elsewhere.
static void InstallTestMobileCasting(DWORD baseAddress)
{
    char path[MAX_PATH] = {};
    if (!GetModuleFileNameA(nullptr, path, MAX_PATH)) return;
    char* slash = strrchr(path, '\\');
    if (!slash || slash - path + 33 >= MAX_PATH) return;
    strcpy_s(slash + 1, MAX_PATH - (slash + 1 - path), "eqdream_mobile_casting.test");
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) return;

    // In the cast-completion check, zero only horizontal displacement used
    // for channeling. Keep the posture gate before 43A182 and hit-count
    // branch at 43A1ED, plus mana/reagents/targeting/cancel logic intact.
    // x87 fsub st(0),st(0) produces zero without changing stack depth.
    // Moving-platform velocity substitutes use fldz with the same depth.
    struct Patch { DWORD va; unsigned char before[3]; unsigned char after[3]; };
    const Patch patches[] = {
        {0x43A188, {0xD8,0x63,0x20}, {0xD8,0xE0,0x90}},
        {0x43A192, {0xD8,0x63,0x24}, {0xD8,0xE0,0x90}},
        {0x43A1A5, {0xD9,0x40,0x70}, {0xD9,0xEE,0x90}},
        {0x43A1AC, {0xD9,0x40,0x74}, {0xD9,0xEE,0x90}},
    };
    // Validate the entire patch set before writing anything.
    for (const auto& p : patches) {
        const void* address = (const void*)(baseAddress + p.va - 0x400000);
        if (memcmp(address, p.before, 3) != 0) {
            OutputDebugStringA("EQDream test mobile casting: unsupported bytes; patch skipped.\n");
            return;
        }
    }
    extern void PatchA(LPVOID address, const void* value, SIZE_T bytes);
    for (const auto& p : patches) {
        void* address = (void*)(baseAddress + p.va - 0x400000);
        PatchA(address, p.after, 3);
        FlushInstructionCache(GetCurrentProcess(), address, 3);
    }
    OutputDebugStringA("EQDream test mobile casting: displacement checks disabled.\n");
}
