#pragma once

#include "MQ2Main.h"
#include <cstdint>

// Presentation for the test-only Spell Shard Forge. The server owns every
// binding, rank, and Spell Shard transaction; the client only displays it.
class NMSSpellShardWnd : public CCustomWnd {
public:
    NMSSpellShardWnd();
    ~NMSSpellShardWnd();

    int WndNotification(CXWnd* pWnd, unsigned int message, void* data);

    static void Initialize();
    static void Shutdown();
    static void OnIncomingPacket(uint16_t opcode, void* buffer, size_t size);
    static void RequestRefresh();

private:
    struct Slot {
        uint16_t spell_id = 0;
        uint8_t rank = 0;
        char name[65] = {};
    };

    static NMSSpellShardWnd* s_instance;
    void ApplySync(const void* buffer, size_t size);
    void RefreshUI();
    void SendRequest(uint8_t action, uint8_t ranks = 1);

    CXWnd* m_slots[10] = {};
    CXWnd* m_nodes[11] = {};
    DWORD m_pending_since = 0;
    CXWnd* m_selected_name = nullptr;
    CXWnd* m_rank = nullptr;
    CXWnd* m_power = nullptr;
    CXWnd* m_duration = nullptr;
    CXWnd* m_resist = nullptr;
    CXWnd* m_cast = nullptr;
    CXWnd* m_pet_info[3] = {};
    CXWnd* m_eom = nullptr;
    CXWnd* m_status = nullptr;
    CButtonWnd* m_bind = nullptr;
    CButtonWnd* m_upgrade = nullptr;
    CButtonWnd* m_unbind = nullptr;
    CButtonWnd* m_refresh = nullptr;
    CComboWnd* m_gem_choice = nullptr;
    Slot m_data[10] = {};
    uint8_t m_selected_slot = 0;
    uint32_t m_spell_shards = 0;
    uint32_t m_request_id = 1;
};

void CmdNMSSpellShard(PSPAWNINFO character, PCHAR arguments);
