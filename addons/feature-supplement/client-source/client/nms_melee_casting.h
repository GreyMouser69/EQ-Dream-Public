#pragma once

// RoF2 local-test opt-in. Never clears casting state or spoofs character class.
static void InstallTestMeleeCasting(DWORD baseAddress)
{
    char path[MAX_PATH] = {};
    if (!GetModuleFileNameA(nullptr, path, MAX_PATH)) return;
    char* slash = strrchr(path, '\\');
    if (!slash || slash - path + 33 >= MAX_PATH) return;
    strcpy_s(slash + 1, MAX_PATH - (slash + 1 - path), "eqdream_melee_casting.test");
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) return;

    struct Patch { DWORD va; unsigned char before[2]; unsigned char after[2]; };
    const Patch patches[] = {
        // EQPlayer melee eligibility: bypass ONLY casting/Bard condition.
        // Continue at 5A0615 through posture, stun and remaining validity checks.
        {0x5A060A, {0x74,0x09}, {0xEB,0x09}},
        // Spell-present branch of animation helper. Null-spell melee animation
        // branch is unchanged. Skip pose dispatch, retain duration processing.
        {0x58F55C, {0xEB,0x26}, {0xEB,0x70}},
        // Begin-cast song pose: skip only actor dispatch, after state updates.
        // Destination is the original epilogue; no argument pushes were made.
        {0x5228ED, {0x8B,0xCD}, {0xEB,0x2C}},
        // End-of-cast pose: preserve target/particle handling at 593DF5 onward.
        {0x593DB0, {0x75,0x43}, {0xEB,0x43}},
    };
    for (const auto& p : patches) {
        const void* address = (const void*)(baseAddress + p.va - 0x400000);
        if (memcmp(address, p.before, 2) != 0) {
            OutputDebugStringA("EQDream test melee casting: unsupported bytes; entire patch skipped.\n");
            return;
        }
    }
    extern void PatchA(LPVOID address, const void* value, SIZE_T bytes);
    for (const auto& p : patches) {
        void* address = (void*)(baseAddress + p.va - 0x400000);
        PatchA(address, p.after, 2);
        FlushInstructionCache(GetCurrentProcess(), address, 2);
    }
    OutputDebugStringA("EQDream test melee casting: client melee gate and spell poses patched.\n");
}
