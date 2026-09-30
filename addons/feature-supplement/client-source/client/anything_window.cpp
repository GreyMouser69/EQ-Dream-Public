#include "anything_window.h"

#include <cstring>
#include <ctime>

extern void AddXMLFile(const char* filename);

namespace {
// Reserved immediately after the current NMS loot opcodes.  The server must
// register the same values before this feature is enabled in a release build.
constexpr uint16_t OP_AnythingSync = 0x140c;
constexpr uint16_t OP_AnythingRequest = 0x140d;
constexpr uint16_t OP_AnythingResult = 0x140e;
// RoF2 server inventory layout: equipment 0-22, packs 23-32, cursor 33.
constexpr uint32_t NativeCursorSlot = 33;

enum AnythingOperation : uint8_t {
    Refresh = 0,
    EquipCursor = 1,
    WithdrawCursor = 2
};

#pragma pack(push, 1)
struct AnythingSlotWire {
    uint8_t slot_index;
    uint8_t occupied;
    uint16_t reserved;
    uint32_t revision;
    uint32_t item_id;
    uint32_t icon;
    int32_t charges;
    uint64_t guid;
    char item_name[64];
};

struct AnythingSyncWire {
    uint16_t version;
    uint16_t slot_count;
    uint32_t character_id;
    uint64_t session_nonce;
    AnythingSlotWire slots[2];
};

struct AnythingRequestWire {
    uint16_t version;
    uint8_t operation;
    uint8_t slot_index;
    uint32_t source_slot;
    uint32_t expected_item_id;
    uint32_t expected_revision;
    uint64_t session_nonce;
    uint64_t request_id;
    uint64_t expected_guid;
};

struct AnythingResultWire {
    uint16_t version;
    uint8_t result;
    uint8_t slot_index;
    uint32_t character_id;
    uint32_t revision;
    uint64_t request_id;
    AnythingSlotWire slots[2];
};
#pragma pack(pop)

static_assert(sizeof(AnythingRequestWire) == 40, "Anything request ABI changed");

}

AnythingWnd* AnythingWnd::s_instance = nullptr;

void CmdAnything(PSPAWNINFO, PCHAR) {
    AnythingRequestWire request = {};
    request.version = 1;
    request.operation = Refresh;
    request.slot_index = 0;
    SendEQMessage(OP_AnythingRequest, &request, sizeof(request));
}

AnythingWnd* AnythingWnd::GetInstance() { return s_instance; }

AnythingWnd::AnythingWnd() : CCustomWnd("AnythingWnd") {
    std::memset(m_slot_names, 0, sizeof(m_slot_names));
    std::memset(m_equip_buttons, 0, sizeof(m_equip_buttons));
    std::memset(m_remove_buttons, 0, sizeof(m_remove_buttons));
    std::memset(m_item_ids, 0, sizeof(m_item_ids));
    std::memset(m_revisions, 0, sizeof(m_revisions));
    std::memset(m_guids, 0, sizeof(m_guids));
    m_session_nonce = 0;
    // Audit request IDs must remain unique across complete client restarts.
    // Starting at one caused a new session to collide with older successful
    // audit rows and roll back otherwise-valid withdraw/equip transactions.
    m_next_request_id = (static_cast<uint64_t>(std::time(nullptr)) << 24) |
        static_cast<uint64_t>(GetTickCount());

    m_slot_names[0] = GetChildItem("AnythingName0");
    m_slot_names[1] = GetChildItem("AnythingName1");
    m_equip_buttons[0] = (CButtonWnd*)GetChildItem("AnythingEquip0");
    m_equip_buttons[1] = (CButtonWnd*)GetChildItem("AnythingEquip1");
    m_remove_buttons[0] = (CButtonWnd*)GetChildItem("AnythingRemove0");
    m_remove_buttons[1] = (CButtonWnd*)GetChildItem("AnythingRemove1");
    SetWndNotification(AnythingWnd);
}

AnythingWnd::~AnythingWnd() = default;

void AnythingWnd::Initialize() { AddXMLFile("EQUI_AnythingWnd.xml"); }

void AnythingWnd::Shutdown() {
    if (!s_instance) return;
    ((CXWnd*)s_instance)->Show(false, false);
    delete s_instance;
    s_instance = nullptr;
}

void AnythingWnd::OnIncomingPacket(uint16_t opcode, void* buffer, size_t size) {
    if ((opcode != OP_AnythingSync && opcode != OP_AnythingResult) || !buffer) return;
    if (!pSidlMgr->FindScreenPieceTemplate("AnythingWnd")) {
        WriteChatf("[Anything] UI template missing. Reload your UI after patching.");
        return;
    }
    if (!s_instance) s_instance = new AnythingWnd();
    if (opcode == OP_AnythingSync) {
        s_instance->ApplySync(buffer, size);
    } else if (size >= sizeof(AnythingResultWire)) {
        const auto* result = static_cast<const AnythingResultWire*>(buffer);
        if (result->version == 1) {
            if (result->result != 0) {
                static const char* messages[] = {
                    "Success", "Malformed request", "Unsupported client version", "Invalid slot",
                    "Session expired; reopen /anything", "Request was stale", "Cursor item changed",
                    "That Anything slot is occupied", "That Anything slot is empty",
                    "Clear your cursor first", "Containers cannot be equipped here",
                    "Database error; please try again", "Anything slots are busy; please try again"
                };
                const char* message = result->result < (sizeof(messages) / sizeof(messages[0]))
                    ? messages[result->result] : "Unknown server error";
                WriteChatf("[Anything] Equip failed: %s.", message);
            }
            AnythingSyncWire sync = {};
            sync.version = 1;
            sync.slot_count = 2;
            sync.character_id = result->character_id;
            sync.session_nonce = s_instance->m_session_nonce;
            sync.slots[0] = result->slots[0];
            sync.slots[1] = result->slots[1];
            s_instance->ApplySync(&sync, sizeof(sync));
        }
    }
    ((CXWnd*)s_instance)->Show(true, true);
}

void AnythingWnd::ApplySync(const void* buffer, size_t size) {
    if (size < sizeof(AnythingSyncWire)) return;
    const auto* sync = static_cast<const AnythingSyncWire*>(buffer);
    if (sync->version != 1 || sync->slot_count > 2) return;
    m_session_nonce = sync->session_nonce;

    for (int i = 0; i < 2; ++i) {
        if (!m_slot_names[i]) continue;
        const bool occupied = i < sync->slot_count && sync->slots[i].occupied != 0;
        m_item_ids[i] = occupied ? sync->slots[i].item_id : 0;
        m_revisions[i] = sync->slots[i].revision;
        m_guids[i] = occupied ? sync->slots[i].guid : 0;
        const char* text = occupied
            ? sync->slots[i].item_name : "Empty";
        CXStr value((PCHAR)text);
        m_slot_names[i]->SetWindowTextA(value);
    }
}

void AnythingWnd::SendRequest(uint8_t operation, uint8_t slot_index) {
    AnythingRequestWire request = {};
    request.version = 1;
    request.operation = operation;
    request.slot_index = slot_index;
    request.source_slot = operation == EquipCursor ? NativeCursorSlot : 0;
    request.expected_item_id = m_item_ids[slot_index];
    request.expected_revision = m_revisions[slot_index];
    request.session_nonce = m_session_nonce;
    request.request_id = m_next_request_id++;
    request.expected_guid = m_guids[slot_index];
    if (operation == EquipCursor) {
        PCHARINFO2 character = GetCharInfo2();
        PCONTENTS cursor = character && character->pInventoryArray
            ? character->pInventoryArray->Inventory.Cursor : nullptr;
        _ITEMINFO* item = cursor ? GetItemFromContents(cursor) : nullptr;
        if (!item) {
            WriteChatf("[Anything] Put the item you want to equip on your cursor first.");
            return;
        }
        request.expected_item_id = item->ItemNumber;
        // RoF2 does not expose the server item serial in this client structure.
        // The server validates the locked cursor database row and its own live
        // ItemInstance before moving anything.
        request.expected_guid = 0;
    }
    SendEQMessage(OP_AnythingRequest, &request, sizeof(request));
}

int AnythingWnd::WndNotification(CXWnd* pWnd, unsigned int message, void* data) {
    if (message == XWM_LCLICK) {
        for (uint8_t i = 0; i < 2; ++i) {
            if (pWnd == (CXWnd*)m_equip_buttons[i]) {
                SendRequest(EquipCursor, i);
                return 1;
            }
            if (pWnd == (CXWnd*)m_remove_buttons[i]) {
                SendRequest(WithdrawCursor, i);
                return 1;
            }
        }
    }
    return CSidlScreenWnd::WndNotification(pWnd, message, data);
}
