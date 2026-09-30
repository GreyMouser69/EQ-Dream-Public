#pragma once

#include "MQ2Main.h"
#include <cstddef>
#include <cstdint>

// Client presentation for the two server-owned Anything slots.  These are
// deliberately not mapped to native EQ inventory slot numbers.
class AnythingWnd : public CCustomWnd {
public:
    AnythingWnd();
    ~AnythingWnd();

    int WndNotification(CXWnd* pWnd, unsigned int message, void* data);

    static void Initialize();
    static void Shutdown();
    static void OnIncomingPacket(uint16_t opcode, void* buffer, size_t size);
    static AnythingWnd* GetInstance();

private:
    static AnythingWnd* s_instance;

    void ApplySync(const void* buffer, size_t size);
    void SendRequest(uint8_t operation, uint8_t slot_index);

    CXWnd* m_slot_names[2];
    CButtonWnd* m_equip_buttons[2];
    CButtonWnd* m_remove_buttons[2];
    uint32_t m_item_ids[2];
    uint32_t m_revisions[2];
    uint64_t m_guids[2];
    uint64_t m_session_nonce;
    uint64_t m_next_request_id;
};

// Opens/refetches the server-authoritative Anything slots.
void CmdAnything(PSPAWNINFO character, PCHAR arguments);
