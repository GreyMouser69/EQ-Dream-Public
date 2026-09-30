#include "nms_spell_shard_window.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

extern void AddXMLFile(const char* filename);

namespace {
constexpr uint16_t OP_NMSSpellShardRequest = 0x140f;
constexpr uint16_t OP_NMSSpellShardSync = 0x1410;
constexpr uint16_t ProtocolVersion = 2;
constexpr uint8_t Refresh = 0;
constexpr uint8_t BindGem = 1;
constexpr uint8_t Upgrade = 2;
constexpr uint8_t Unbind = 3;

#pragma pack(push, 1)
struct RequestWire {
    uint16_t protocol_version;
    uint8_t action;
    uint8_t slot_index;
    uint8_t gem_index;
    uint8_t ranks;
    uint32_t request_id;
};
struct SlotWire {
    uint16_t spell_id;
    uint8_t rank;
    uint8_t slot_index;
    char spell_name[64];
};
struct SyncWire {
    uint16_t protocol_version;
    uint8_t status;
    uint8_t action;
    uint32_t spell_shards;
    uint32_t request_id;
    SlotWire slots[10];
};
#pragma pack(pop)

static_assert(sizeof(RequestWire) == 10, "Spell Shard request ABI changed");

const char* StatusText(uint8_t status) {
    static const char* messages[] = {
        "Forge ready.", "Invalid request.", "Client needs an update.",
        "Invalid forge slot.", "Invalid spell gem.", "That spell cannot be mastered.",
        "Bind a spell into this forge slot first.", "That spell is already rank +10.",
        "You need more Spell Shards."
    };
    return status < sizeof(messages) / sizeof(messages[0]) ? messages[status] : "Forge request failed.";
}

unsigned PetLevelBonus(unsigned rank) {
    rank = (std::min)(rank, 10u);
    return rank + (rank > 5 ? rank - 5 : 0);
}

bool IsMasteryPetSpell(uint16_t id) {
    auto spell = GetSpellByID(id);
    if (!spell) return false;
    for (int i = 0; i < 12; ++i) {
        // Summon Pet, Necromancer Pet, Beastlord Pet (not familiars/swarm spells).
        if (spell->Attrib[i] == 33 || spell->Attrib[i] == 71 || spell->Attrib[i] == 106) return true;
    }
    return false;
}

void SetText(CXWnd* control, const char* text) {
    if (control) control->SetWindowTextA(CXStr((PCHAR)text));
}
}

NMSSpellShardWnd* NMSSpellShardWnd::s_instance = nullptr;

void CmdNMSSpellShard(PSPAWNINFO, PCHAR) { NMSSpellShardWnd::RequestRefresh(); }

NMSSpellShardWnd::NMSSpellShardWnd() : CCustomWnd("NMSSpellShardWnd") {
    char id[64] = {};
    for (int i = 0; i < 10; ++i) {
        std::snprintf(id, sizeof(id), "SpellShardSlot%d", i);
        m_slots[i] = GetChildItem(id);
    }
    m_selected_name = GetChildItem("SpellShardSelectedName");
    for (int i=0; i<=10; ++i) {
        std::snprintf(id,sizeof(id),"SpellShardLit%d",i);
        m_nodes[i]=GetChildItem(id);
    }
    m_rank = GetChildItem("SpellShardRank");
    m_power = GetChildItem("SpellShardPower");
    m_duration = GetChildItem("SpellShardDuration");
    m_resist = GetChildItem("SpellShardResist");
    m_cast = GetChildItem("SpellShardCast");
    for (int i=0;i<3;++i) {
        std::snprintf(id,sizeof(id),"SpellShardPetInfo%d",i);
        m_pet_info[i] = GetChildItem(id);
    }
    m_eom = GetChildItem("SpellShardEOM");
    m_status = GetChildItem("SpellShardStatus");
    m_bind = (CButtonWnd*)GetChildItem("SpellShardBind");
    m_upgrade = (CButtonWnd*)GetChildItem("SpellShardUpgrade");
    m_unbind = (CButtonWnd*)GetChildItem("SpellShardUnbind");
    m_refresh = (CButtonWnd*)GetChildItem("SpellShardRefresh");
    m_gem_choice = (CComboWnd*)GetChildItem("SpellShardGemChoice");
    if (m_gem_choice) {
        for (int gem = 1; gem <= 12; ++gem) {
            std::snprintf(id, sizeof(id), "Memorized Gem %d", gem);
            m_gem_choice->InsertChoice(id);
        }
        m_gem_choice->SetChoice(0);
    }
    SetWndNotification(NMSSpellShardWnd);
}

NMSSpellShardWnd::~NMSSpellShardWnd() = default;

void NMSSpellShardWnd::Initialize() { /* Installed through EQUI.xml, like the loot window. */ }

void NMSSpellShardWnd::Shutdown() {
    if (!s_instance) return;
    ((CXWnd*)s_instance)->Show(false, false);
    delete s_instance;
    s_instance = nullptr;
}

void NMSSpellShardWnd::RequestRefresh() {
    RequestWire request = {};
    request.protocol_version = ProtocolVersion;
    request.action = Refresh;
    SendEQMessage(OP_NMSSpellShardRequest, &request, sizeof(request));
}

void NMSSpellShardWnd::OnIncomingPacket(uint16_t opcode, void* buffer, size_t size) {
    if (opcode != OP_NMSSpellShardSync || !buffer || size != sizeof(SyncWire)) return;
    if (static_cast<const SyncWire*>(buffer)->protocol_version != ProtocolVersion) return;
    if (!pSidlMgr->FindScreenPieceTemplate("NMSSpellShardWnd")) {
        WriteChatf("[Spell Shards] UI template missing. Reload your UI after patching.");
        return;
    }
    if (!s_instance) s_instance = new NMSSpellShardWnd();
    s_instance->ApplySync(buffer, size);
    ((CXWnd*)s_instance)->Show(true, true);
}

void NMSSpellShardWnd::ApplySync(const void* buffer, size_t size) {
    if (size < sizeof(SyncWire)) return;
    const auto* sync = static_cast<const SyncWire*>(buffer);
    if (sync->protocol_version != ProtocolVersion) return;
    m_spell_shards = sync->spell_shards;
    m_pending_since = 0;
    for (int i = 0; i < 10; ++i) {
        m_data[i].spell_id = sync->slots[i].spell_id;
        m_data[i].rank = (std::min)(uint8_t(10), sync->slots[i].rank);
        std::memset(m_data[i].name, 0, sizeof(m_data[i].name));
        std::memcpy(m_data[i].name, sync->slots[i].spell_name, sizeof(sync->slots[i].spell_name));
    }
    SetText(m_status, StatusText(sync->status));
    RefreshUI();
}

void NMSSpellShardWnd::RefreshUI() {
    char text[256] = {};
    for (int i = 0; i < 10; ++i) {
        if (m_data[i].spell_id) {
            std::snprintf(text, sizeof(text), "%d. %s  [+%u]", i + 1, m_data[i].name, m_data[i].rank);
        } else {
            std::snprintf(text, sizeof(text), "%d. Empty spell shard slot", i + 1);
        }
        SetText(m_slots[i], text);
    }
    const auto& selected = m_data[m_selected_slot];
    for (int i=0;i<=10;++i) if(m_nodes[i]) m_nodes[i]->Show(selected.spell_id && i<=selected.rank,false);
    if (!selected.spell_id) {
        SetText(m_selected_name, "SELECT A FORGE SLOT");
        SetText(m_rank, "Rank: - / +10");
        SetText(m_power, "Power: bind a memorized spell");
        SetText(m_duration, "Duration: +2% per rank");
        SetText(m_resist, "Resist difficulty: -40 per rank");
        SetText(m_cast, "Cast time: -0.5 seconds per rank, maximum -5.0 seconds");
    } else {
        SetText(m_selected_name, selected.name);
        std::snprintf(text, sizeof(text), "Spell Shard Mastery: +%u / +10", selected.rank);
        SetText(m_rank, text);
        if (IsMasteryPetSpell(selected.spell_id)) {
            std::snprintf(text, sizeof(text), "HP/DMG/AC/resists: +%u%% -> +%u%%", selected.rank * 50, (std::min)(10, selected.rank + 1) * 50);
            SetText(m_power, text);
            std::snprintf(text, sizeof(text), "Pet level +%u; STR/STA/DEX/AGI +%u%%", PetLevelBonus(selected.rank), selected.rank * 50);
            SetText(m_duration, text);
            std::snprintf(text, sizeof(text), "ATK/Accuracy/EVA: +%u%% -> +%u%%", selected.rank * 50, (std::min)(10, selected.rank + 1) * 50);
            SetText(m_resist, text);
        } else {
        std::snprintf(text, sizeof(text), "Power: +%u%%   -> next +%u%%", selected.rank * 2, (std::min)(10, selected.rank + 1) * 2);
        SetText(m_power, text);
        std::snprintf(text, sizeof(text), "Duration: +%u%%   -> next +%u%%", selected.rank * 2, (std::min)(10, selected.rank + 1) * 2);
        SetText(m_duration, text);
        std::snprintf(text, sizeof(text), "Resist difficulty: -%u   -> next -%u", selected.rank * 40, (std::min)(10, selected.rank + 1) * 40);
        SetText(m_resist, text);
        }
        std::snprintf(text, sizeof(text), "Cast time: -%.1f sec   -> next -%.1f sec (cap 5.0)", selected.rank * 0.5f, (std::min)(10, selected.rank + 1) * 0.5f);
        SetText(m_cast, text);
    }
    const bool pet_selected = selected.spell_id && IsMasteryPetSpell(selected.spell_id);
    for (int i=0;i<3;++i) {
        SetText(m_pet_info[i], "");
        if (m_pet_info[i]) m_pet_info[i]->Show(pet_selected,false);
    }
    if (pet_selected) {
        std::snprintf(text,sizeof(text),"PET RANK +%u: %.1fx stats / +%u levels",selected.rank,1.0f+selected.rank*0.5f,PetLevelBonus(selected.rank));
        SetText(m_pet_info[0],text);
        if (selected.rank<10) {
            std::snprintf(text,sizeof(text),"Next +%u: %.1fx / +%u levels | %u shards",selected.rank+1,1.0f+(selected.rank+1)*0.5f,PetLevelBonus(selected.rank+1),1u<<selected.rank);
            SetText(m_pet_info[1],text);
        } else SetText(m_pet_info[1],"MAX RANK: +500% stats and +15 levels");
        SetText(m_pet_info[2],"After focus. Resummon after upgrading.");
    }
    std::snprintf(text, sizeof(text), "Spell Shards: %u", m_spell_shards);
    SetText(m_eom, text);
    if (!selected.spell_id) SetText((CXWnd*)m_upgrade, "SELECT A SPELL TO UPGRADE");
    else if (selected.rank >= 10) SetText((CXWnd*)m_upgrade, "MAXIMUM RANK REACHED");
    else {
        std::snprintf(text, sizeof(text), "UPGRADE +%u - %u SHARDS", selected.rank + 1, 1u << selected.rank);
        SetText((CXWnd*)m_upgrade, text);
    }
}

void NMSSpellShardWnd::SendRequest(uint8_t action, uint8_t ranks) {
    if(action != Refresh && m_pending_since && GetTickCount()-m_pending_since<5000) return;
    if(action == Upgrade && (!m_data[m_selected_slot].spell_id || m_data[m_selected_slot].rank>=10 || m_spell_shards < (1u << m_data[m_selected_slot].rank))) {
        SetText(m_status,"Select a spell below rank +10 and have enough Spell Shards for its next rank.");
        return;
    }
    RequestWire request = {};
    request.protocol_version = ProtocolVersion;
    request.action = action;
    request.slot_index = m_selected_slot;
    request.gem_index = m_gem_choice ? (uint8_t)(std::max)(0, m_gem_choice->GetCurChoice()) : 0;
    request.ranks = ranks;
    request.request_id = m_request_id++;
    m_pending_since = GetTickCount();
    SetText(m_status,"Waiting for the Forge...");
    SendEQMessage(OP_NMSSpellShardRequest, &request, sizeof(request));
}

int NMSSpellShardWnd::WndNotification(CXWnd* pWnd, unsigned int message, void* data) {
    if (message == XWM_LCLICK) {
        for (uint8_t i = 0; i < 10; ++i) {
            if (pWnd == m_slots[i]) { m_selected_slot = i; RefreshUI(); return 1; }
        }
        if (pWnd == (CXWnd*)m_bind) { SendRequest(BindGem); return 1; }
        if (pWnd == (CXWnd*)m_upgrade) { SendRequest(Upgrade); return 1; }
        if (pWnd == (CXWnd*)m_unbind) { SendRequest(Unbind); return 1; }
        if (pWnd == (CXWnd*)m_refresh) { SendRequest(Refresh); return 1; }
    }
    return CSidlScreenWnd::WndNotification(pWnd, message, data);
}
