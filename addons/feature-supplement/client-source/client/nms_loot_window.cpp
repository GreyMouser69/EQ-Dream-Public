#include "nms_loot_window.h"
#include "nms_loot_choices.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <windows.h>

extern void AddXMLFile(const char* filename);

namespace {
constexpr size_t LootVisibleRows = 5;
static_assert(offsetof(CSIDLWND,VScrollPos)==0x17c,"RoF2 scroll position layout changed");
static_assert(offsetof(CSIDLWND,VScrollMax)==0x190,"RoF2 scroll range layout changed");
constexpr uint16_t OP_NMSLootOffer = 0x140a;
constexpr uint16_t OP_NMSLootResponse = 0x140b;

#pragma pack(push, 1)
struct OfferHeader {
    uint16_t corpse_id;
    char looter_name[64];
    uint32_t character_id;
    uint32_t zone_id;
    uint32_t instance_id;
    uint32_t reserved;
    uint32_t item_count;
};
struct OfferItem {
    uint32_t item_id;
    uint32_t icon;
    uint32_t quantity;
    uint32_t loot_slot;
    uint32_t is_bonus_item;
    uint16_t flags;
    char item_name[64];
    char source_name[64];
};
struct LootResponse {
    uint16_t corpse_id;
    uint32_t loot_slot;
    uint32_t item_id;
    uint8_t action;
    char item_name[64];
};
#pragma pack(pop)

static_assert(sizeof(OfferHeader) == 0x56, "NMS offer header ABI changed");
static_assert(sizeof(OfferItem) == 0x96, "NMS offer item ABI changed");
}

NMSLootWnd* NMSLootWnd::s_instance = nullptr;

void CmdNMSLoot(PSPAWNINFO, PCHAR) { NMSLootWnd::ToggleWindow(); }

NMSLootWnd::NMSLootWnd()
    : CCustomWnd("NMSLootWnd"), m_keep_all(nullptr), m_sell_all(nullptr),
      m_loot_all(nullptr), m_close(nullptr) {
    std::memset(m_names, 0, sizeof(m_names));
    std::memset(m_slots, 0, sizeof(m_slots));
    char id[64];
    for (int i = 0; i < 12; ++i) {
        std::snprintf(id,sizeof(id),"NMSLoot_SoloMenu%d",i);m_solo_menu[i]=(CComboWnd*)GetChildItem(id);
        std::snprintf(id,sizeof(id),"NMSLoot_GroupMenu%d",i);m_group_menu[i]=(CComboWnd*)GetChildItem(id);
        std::snprintf(id,sizeof(id),"NMSLoot_Voted%d",i);m_voted[i]=GetChildItem(id);
        std::snprintf(id,sizeof(id),"NMSLoot_Auto%d",i);m_auto[i]=GetChildItem(id);
        std::snprintf(id, sizeof(id), "NMSLoot_Slot%d", i); m_slots[i] = GetChildItem(id);
        std::snprintf(id,sizeof(id),"NMSLoot_Name%d_Gear",i);m_gear_names[i]=GetChildItem(id);
        std::snprintf(id,sizeof(id),"NMSLoot_Name%d_Quest",i);m_quest_names[i]=GetChildItem(id);
        std::snprintf(id, sizeof(id), "NMSLoot_Name%d", i); m_names[i] = GetChildItem(id);
    }
    m_keep_all = (CButtonWnd*)GetChildItem("NMSLoot_KeepAllButton");
    m_sell_all = (CButtonWnd*)GetChildItem("NMSLoot_SellAllButton");
    m_loot_all = (CButtonWnd*)GetChildItem("NMSLoot_LootAllButton");
    m_close = (CButtonWnd*)GetChildItem("NMSLoot_CloseButton");
    m_rule_list = (CListWnd*)GetChildItem("NMSLoot_WildcardList");
    m_rule_input = GetChildItem("NMSLoot_WildcardInput");
    m_rule_add = GetChildItem("NMSLoot_AddWildcardButton");
    m_rule_delete = GetChildItem("NMSLoot_DeleteWildcardButton");
    const char* controls[] = {"NMSLoot_WKeep", "NMSLoot_WSell", "NMSLoot_WDestroy", "NMSLoot_WBank", "NMSLoot_WVault", "NMSLoot_WTribute"};
    for (int i=0;i<6;++i) m_rule_actions[i]=GetChildItem((PCHAR)controls[i]);
    m_group_actions[0]=GetChildItem("NMSLoot_RuleNeed");m_group_actions[1]=GetChildItem("NMSLoot_RuleGreed");m_group_actions[2]=GetChildItem("NMSLoot_RulePass");
    m_group_actions[3]=GetChildItem("NMSLoot_RuleGroupSell");
    m_search_input=GetChildItem("NMSLoot_RuleSearch");
    m_search_go=GetChildItem("NMSLoot_RuleSearchGo");
    m_search_clear=GetChildItem("NMSLoot_RuleSearchClear");
    m_rule_category=(CComboWnd*)GetChildItem("NMSLoot_RuleCategory");
    m_rule_count=GetChildItem("NMSLoot_RuleCount");
    if(m_rule_category) m_rule_category->SetChoice(0);
    m_scrollbar=GetChildItem("NMSLoot_Scrollbar");
    m_item_count=GetChildItem("NMSLoot_ItemCount");
    InstallScrollbar();
    SetWndNotification(NMSLootWnd);
    const RECT rect=((PCSIDLWND)this)->Location;
    const int x=(std::max)(0L,rect.left),y=(std::max)(0L,rect.top);
    ((CXWnd*)this)->Move(CXRect(x,y,x+420,y+420));
}

NMSLootWnd::~NMSLootWnd() {
    if(m_scrollbar && m_scroll_original) ((PCSIDLWND)m_scrollbar)->pvfTable=m_scroll_original;
    delete m_scroll_table;
}
void NMSLootWnd::Initialize() { /* EQUI.xml includes the loot template; do not compete with external MacroQuest XML injection. */ }

void NMSLootWnd::Shutdown() {
    if (!s_instance) return;
    ((CXWnd*)s_instance)->Show(false, false);
    delete s_instance;
    s_instance = nullptr;
}

void NMSLootWnd::ToggleWindow() {
    if (!pSidlMgr->FindScreenPieceTemplate("NMSLootWnd")) {
        WriteChatf("[NMS] UI template missing. Reload the default UI after patching.");
        return;
    }
    if (!s_instance) s_instance = new NMSLootWnd();
    const bool repositioned = s_instance->EnsureOnScreen();
    const bool show = repositioned || !((CXWnd*)s_instance)->IsReallyVisible();
    ((CXWnd*)s_instance)->Show(show, true);
    if (show) s_instance->RefreshUI();
}

std::string NMSLootWnd::IniPath() const {
    const char* name = GetCharInfo() ? GetCharInfo()->Name : "Unknown";
    char dir[MAX_PATH];
    char path[MAX_PATH];
    std::snprintf(dir, sizeof(dir), "%s\\NMSLoot", gszINIPath);
    CreateDirectoryA(dir, nullptr);
    std::snprintf(path, sizeof(path), "%s\\%s_LootList.ini", dir, name);
    return path;
}

std::string NMSLootWnd::RuleFor(const PendingItem& item) const {
    char value[256] = {0};
    GetPrivateProfileStringA("Rules", item.name.c_str(), "", value, sizeof(value), IniPath().c_str());
    char* separator = std::strchr(value, '|');
    if (separator) *separator = 0;
    if (!*value) {
        char filters[32768] = {};
        GetPrivateProfileSectionA("Filters", filters, sizeof(filters), IniPath().c_str());
        std::string name=item.name;
        std::transform(name.begin(),name.end(),name.begin(),[](unsigned char c){return (char)std::tolower(c);});
        for (const char* p=filters;*p;p+=std::strlen(p)+1) {
            std::string entry=p;auto equal=entry.find('=');if(equal==std::string::npos || equal==0) continue;
            std::string term=entry.substr(0,equal);
            std::transform(term.begin(),term.end(),term.begin(),[](unsigned char c){return (char)std::tolower(c);});
            if(name.find(term)!=std::string::npos) return entry.substr(equal+1);
        }
    }
    return value;
}

bool NMSLootWnd::EnsureOnScreen() {
    if (!pScreenXMax || !pScreenYMax || ScreenXMax < 100 || ScreenYMax < 100) return false;
    const RECT r=((PCSIDLWND)this)->Location;
    const int width=(std::max)(100L,r.right-r.left), height=(std::max)(100L,r.bottom-r.top);
    const int x=(std::max)(0,(std::min)((int)r.left,(std::max)(0,(int)ScreenXMax-width)));
    const int y=(std::max)(0,(std::min)((int)r.top,(std::max)(0,(int)ScreenYMax-height)));
    if(x==r.left && y==r.top) return false;
    ((CXWnd*)this)->Move(CXRect(x,y,x+width,y+height));
    return true;
}

void NMSLootWnd::SearchRules(bool clear) {
    if(!m_search_input) return;
    char query[512]={};
    if(clear) m_search_input->SetWindowTextA(CXStr(""));
    else {CXStr text=m_search_input->GetWindowTextA();GetCXStr(text.Ptr,query,sizeof(query));}
    m_rule_search=query;
    std::transform(m_rule_search.begin(),m_rule_search.end(),m_rule_search.begin(),[](unsigned char c){return (char)std::tolower(c);});
    RefreshRules();
}

void NMSLootWnd::RefreshRules() {
    if(!m_rule_list) return;
    m_rule_list->DeleteAll();m_rules.clear();
    size_t total=0;
    for(const char* section : {"Rules","Filters","GroupRules"}) {
        char entries[32768] = {};
        GetPrivateProfileSectionA(section,entries,sizeof(entries),IniPath().c_str());
        for(const char* p=entries;*p;p+=std::strlen(p)+1) {
            std::string entry=p;auto eq=entry.find('=');if(eq==std::string::npos) continue;
            SavedRule rule{section,entry.substr(0,eq),entry.substr(eq+1)};
            ++total;
            if(!LootChoices::Matches(rule.key,rule.value,m_rule_category_choice,m_rule_search)) continue;
            std::string name=(rule.section=="Filters"?"[Contains] ":(rule.section=="GroupRules"?"[Group Auto] ":"[Item] "))+rule.key;
            int row=m_rule_list->AddString(name.c_str(),0xFFFFFF41,(uint32_t)m_rules.size(),0);
            std::string action=rule.value.substr(0,rule.value.find('|'));
            CXStr text((PCHAR)action.c_str());m_rule_list->SetItemText(row,1,&text);
            m_rules.push_back(rule);
        }
    }
    if(m_rule_count) {
        char count[80];std::snprintf(count,sizeof(count),"Showing %u of %u rules",(unsigned)m_rules.size(),(unsigned)total);
        m_rule_count->SetWindowTextA(CXStr(count));
    }
}

void NMSLootWnd::ChangeRule(const char* action) {
    const bool explicitGroupSell=!strcmp(action,"GroupSell");
    if(explicitGroupSell) action="Sell";
    const bool solo=strcmp(action,"Need") && strcmp(action,"Greed") && strcmp(action,"Pass");
    if(solo) {
        m_filter_action=action;
        if(!RuleInputText().empty()) {AddRuleFilter(action);return;}
    }
    int index=m_rule_list?m_rule_list->GetCurSel():-1;
    if(index>=0 && (size_t)index<m_rules.size()) {
        const auto& rule=m_rules[index];
        const bool groupAction=explicitGroupSell || !solo;
        if(groupAction!=(rule.section=="GroupRules")) {WriteChatf("[NMS] Use Need/Greed/Sell/Pass for Group Auto rules, or solo actions for item/filter rules.");return;}
        auto suffix=rule.value.find('|');
        std::string value=std::string(action)+(suffix==std::string::npos?"":rule.value.substr(suffix));
        WritePrivateProfileStringA(rule.section.c_str(),rule.key.c_str(),value.c_str(),IniPath().c_str());
        WriteChatf("[NMS] %s rule '%s' changed to %s.",rule.section=="Filters"?"Filter":(rule.section=="GroupRules"?"Group auto":"Item"),rule.key.c_str(),action);
        RefreshRules();
        return;
    }
    if(solo) WriteChatf("[NMS] New filter rules will use %s. Type a name and click Add Filter Rule.",action);
}

std::string NMSLootWnd::RuleInputText() const {
    if(!m_rule_input) return "";
    char raw[256]={};CXStr text=m_rule_input->GetWindowTextA();GetCXStr(text.Ptr,raw,sizeof(raw));
    return LootChoices::Trim(raw);
}

void NMSLootWnd::AddRuleFilter(const char* requested) {
    std::string term=RuleInputText();
    if(term.empty()) {WriteChatf("[NMS] Type an item name, then click Add Filter Rule (or a rule-action button).");return;}
    if(!LootChoices::ValidName(term)) {WriteChatf("[NMS] Rule names cannot contain = [ ] or line breaks.");return;}
    const std::string action=LootChoices::ResolveAddAction(requested,m_filter_action);
    char existing[128]={};GetPrivateProfileStringA("Filters",term.c_str(),"",existing,sizeof(existing),IniPath().c_str());
    const bool update=*existing!=0;
    WritePrivateProfileStringA("Filters",term.c_str(),action.c_str(),IniPath().c_str());
    m_rule_input->SetWindowTextA(CXStr(""));
    if(m_rule_category) {m_rule_category->SetChoice(0);m_rule_category_choice=0;}
    m_rule_search.clear();
    if(m_search_input) m_search_input->SetWindowTextA(CXStr(""));
    RefreshRules();
    WriteChatf("[NMS] Filter %s: names containing '%s' => %s.",update?"updated":"added",term.c_str(),action.c_str());
}

std::string NMSLootWnd::GroupRuleFor(const PendingItem& item) const {
    char value[256]={};GetPrivateProfileStringA("GroupRules",item.name.c_str(),"",value,sizeof(value),IniPath().c_str());
    std::string rule=value;auto sep=rule.find('|');
    if(sep==std::string::npos || rule.substr(sep+1)!=std::to_string(item.item_id)) return "";
    rule.resize(sep);return rule;
}

void NMSLootWnd::ToggleAuto(size_t index) {
    if(index>=m_pending.size() || !m_pending[index].group_token) return;
    auto& item=m_pending[index];item.remember=!item.remember;
    if(!item.remember) WritePrivateProfileStringA("GroupRules",item.name.c_str(),nullptr,IniPath().c_str());
    RefreshUI();
}

void NMSLootWnd::SaveRule(const PendingItem& item, const char* rule) const {
    char value[128];
    std::snprintf(value, sizeof(value), "%s|%u|%u", rule, item.icon, item.item_id);
    WritePrivateProfileStringA("Rules", item.name.c_str(), value, IniPath().c_str());
}

void NMSLootWnd::SendDecision(const PendingItem& item, uint8_t action) {
    LootResponse response = {};
    response.corpse_id = item.corpse_id;
    response.loot_slot = item.loot_slot;
    response.item_id = item.item_id;
    response.action = action;
    if(item.personal_token) std::snprintf(response.item_name,sizeof(response.item_name),"%u",item.personal_token);
    else if(item.group_token) std::snprintf(response.item_name,sizeof(response.item_name),"%u",item.group_token);
    else strncpy_s(response.item_name, item.name.c_str(), _TRUNCATE);
    SendEQMessage(OP_NMSLootResponse, &response, sizeof(response));
}

void NMSLootWnd::OnIncomingPacket(uint16_t opcode, void* buffer, size_t size) {
    if (opcode != OP_NMSLootOffer || !buffer || size < sizeof(OfferHeader)) return;
    const OfferHeader* header = static_cast<const OfferHeader*>(buffer);
    const size_t available = (size - sizeof(OfferHeader)) / sizeof(OfferItem);
    const size_t count = std::min<size_t>(header->item_count, available);
    const OfferItem* items = reinterpret_cast<const OfferItem*>(static_cast<const char*>(buffer) + sizeof(OfferHeader));
    if (!pSidlMgr->FindScreenPieceTemplate("NMSLootWnd")) {
        WriteChatf("[NMS] UI template missing. Reload the default UI after patching.");
        return;
    }
    if (!s_instance) s_instance = new NMSLootWnd();
    if(header->reserved) {
        auto &rows=s_instance->m_pending;
        rows.erase(std::remove_if(rows.begin(),rows.end(),[header](const PendingItem& p){return p.corpse_id==header->corpse_id;}),rows.end());
    }
    for (size_t i = 0; i < count; ++i) {
        PendingItem pending = {};
        pending.corpse_id = header->corpse_id;
        pending.loot_slot = items[i].loot_slot;
        pending.item_id = items[i].item_id;
        pending.icon = items[i].icon;
        pending.category=items[i].flags & 0x0006;
        pending.name.assign(items[i].item_name, strnlen_s(items[i].item_name, sizeof(items[i].item_name)));
        if(header->reserved && (items[i].flags & 0x8000)) {
            pending.group_token=header->reserved;pending.group_flags=items[i].flags & 0xf000;
            auto saved=s_instance->GroupRuleFor(pending);
            pending.remember=saved=="Need" || saved=="Greed" || saved=="Sell" || saved=="Pass";
            if(pending.group_flags==0x8000 && pending.remember) {
                s_instance->SendDecision(pending,saved=="Need"?10:(saved=="Greed"?11:(saved=="Sell"?15:12)));
                pending.group_flags=0xa000;
            }
            if(pending.group_flags==0x8000 || pending.group_flags==0xa000) s_instance->m_pending.push_back(pending);
            continue;
        }
        if(header->reserved && (items[i].flags & 0x4000)) {
            pending.personal_token=header->reserved;
            s_instance->m_pending.push_back(pending);
            if(!(items[i].flags & 0x2000)) {
                const auto saved=s_instance->RuleFor(pending);
                if(_stricmp(saved.c_str(),"Keep")==0) s_instance->SendDecision(pending,1);
                else if(_stricmp(saved.c_str(),"Sell")==0) s_instance->SendDecision(pending,2);
            }
            continue;
        }
        const std::string rule = s_instance->RuleFor(pending);
        if (_stricmp(rule.c_str(), "Keep") == 0) s_instance->SendDecision(pending, 1);
        else if (_stricmp(rule.c_str(), "Sell") == 0) s_instance->SendDecision(pending, 2);
        else s_instance->m_pending.push_back(pending);
    }
    s_instance->RefreshUI();
    if (!s_instance->m_pending.empty()) {s_instance->EnsureOnScreen();((CXWnd*)s_instance)->Show(true, true);}
}

void NMSLootWnd::RefreshUI() {
    RefreshRules();
    if(m_had_pending && m_pending.empty()) {
        ((CXWnd*)this)->Show(false,false);
    }
    m_had_pending=!m_pending.empty();
    const size_t maximum=m_pending.size()>LootVisibleRows?m_pending.size()-LootVisibleRows:0;
    m_first_item=(std::min)(m_first_item,maximum);
    if(m_scrollbar && m_scroll_table) {
        ((PCSIDLWND)m_scrollbar)->VScrollMax=(DWORD)(maximum*48);
        m_scrollbar->SetVScrollPos((int)m_first_item*48);
        m_scrollbar->Show(maximum>0,false);
    }
    if(m_item_count) {
        char count[80];
        if(m_pending.empty()) std::snprintf(count,sizeof(count),"No pending loot");
        else std::snprintf(count,sizeof(count),"Items %u-%u of %u",(unsigned)m_first_item+1,
            (unsigned)(std::min)(m_first_item+LootVisibleRows,m_pending.size()),(unsigned)m_pending.size());
        m_item_count->SetWindowTextA(CXStr(count));
    }
    auto rank=[](const PendingItem& p) {return !p.group_token?2:(p.group_flags==0xc000?0:(p.group_flags==0xa000?3:1));};
    std::stable_sort(m_pending.begin(),m_pending.end(),[&](const PendingItem& a,const PendingItem& b){return rank(a)<rank(b);});
    for (size_t i = 0; i < 12; ++i) {
        const size_t index=m_first_item+i;
        const bool occupied = i<LootVisibleRows && index < m_pending.size();
        const bool group=occupied && m_pending[index].group_token;
        if(m_auto[i]) {m_auto[i]->Show(group,false);m_auto[i]->SetWindowTextA(CXStr((PCHAR)(group && m_pending[index].remember?"[x] Auto":"[ ] Auto")));}
        PaintItem(m_names[i],m_slots[i],occupied?&m_pending[index]:nullptr,m_gear_names[i],m_quest_names[i]);
        UpdateMenus(m_solo_menu[i],m_group_menu[i],m_voted[i],occupied?&m_pending[index]:nullptr);
    }
}

void NMSLootWnd::Decide(size_t index, uint8_t action, const char* rule) {
    if (index >= m_pending.size()) return;
    PendingItem item = m_pending[index];
    if(item.group_token) {
        if(item.group_flags==0xa000) return;
        if(item.group_flags==0xc000) {if(action==1) SendDecision(item,13);return;}
        if(action!=0 && action!=1 && action!=2 && action!=4) return;
        if(item.remember) {
            std::string saved=std::string(action==1?"Need":(action==2?"Greed":(action==4?"Sell":"Pass")))+"|"+std::to_string(item.item_id);
            WritePrivateProfileStringA("GroupRules",item.name.c_str(),saved.c_str(),IniPath().c_str());
        }
        SendDecision(item,action==1?10:(action==2?11:(action==4?15:12)));
        m_pending[index].group_flags=0xa000; RefreshUI(); return;
    }
    if(item.personal_token) {
        if(action!=1 && action!=2) {
            WriteChatf("[NMS] Personal drops: use Keep or Sell. Other saved rules do not discard your item.");
            return;
        }
        if(rule && *rule) SaveRule(item,rule);
        SendDecision(item,action);
        return; // Keep the row until the server confirms delivery/sale.
    }
    if (rule && *rule) SaveRule(item, rule);
    if (action == 1 || action == 2) SendDecision(item, action);
    m_pending.erase(m_pending.begin() + index);
    RefreshUI();
}

void NMSLootWnd::DecideAll(uint8_t action, const char* rule) {
    for(size_t i=m_pending.size();i>0;--i) if(!m_pending[i-1].group_token) Decide(i-1,action,rule);
}

int NMSLootWnd::WndNotification(CXWnd* pWnd, unsigned int message, void* data) {
    if(message==XWM_HITENTER && pWnd && pWnd==m_search_input) {SearchRules(false);return 1;}
    if(message==XWM_HITENTER && pWnd && pWnd==m_rule_input) {AddRuleFilter(nullptr);return 1;}
    if (message == XWM_LCLICK && pWnd) {
        if(pWnd==(CXWnd*)m_rule_category) {
            const int choice=m_rule_category->GetCurChoice();
            if(choice>=0 && choice<7 && choice!=m_rule_category_choice) {
                m_rule_category_choice=choice;SearchRules(true);
            }
            return 1;
        }
        if(pWnd && pWnd==m_search_go) {SearchRules(false);return 1;}
        if(pWnd && pWnd==m_search_clear) {SearchRules(true);return 1;}
        const char* groupActions[]={"Need","Greed","Pass","GroupSell"};
        for(int i=0;i<4;++i) if(pWnd && pWnd==m_group_actions[i]) {ChangeRule(groupActions[i]);return 1;}
        const char* actions[]={"Keep","Sell","Destroy","Bank","Vault","Tribute"};
        for(int i=0;i<6;++i) if(pWnd==m_rule_actions[i] && pWnd) {ChangeRule(actions[i]);return 1;}
        if(pWnd==m_rule_delete && pWnd) {
            int row=m_rule_list?m_rule_list->GetCurSel():-1;
            if(row>=0 && (size_t)row<m_rules.size()) {auto rule=m_rules[row];WritePrivateProfileStringA(rule.section.c_str(),rule.key.c_str(),nullptr,IniPath().c_str());RefreshRules();}
            return 1;
        }
        if(pWnd==m_rule_add && pWnd) {AddRuleFilter(nullptr);return 1;}
        for (size_t i = 0; i < LootVisibleRows; ++i) {
            const size_t index=m_first_item+i;
            if(pWnd && (pWnd==(CXWnd*)m_solo_menu[i] || pWnd==(CXWnd*)m_group_menu[i])) {ChooseMenu(index,(CComboWnd*)pWnd,pWnd==(CXWnd*)m_group_menu[i]);return 1;}
            if(pWnd && pWnd==m_auto[i]) {ToggleAuto(index);return 1;}
            if ((pWnd == m_names[i] || pWnd == m_gear_names[i] || pWnd == m_quest_names[i] || pWnd == m_slots[i]) && index < m_pending.size()) {
                SendDecision(m_pending[index], 14); // Read-only native item inspection.
                return 1;
            }
        }
        if (pWnd == (CXWnd*)m_keep_all || pWnd == (CXWnd*)m_loot_all) { DecideAll(1, nullptr); return 1; }
        if (pWnd == (CXWnd*)m_sell_all) { DecideAll(2, nullptr); return 1; }
        if (pWnd == (CXWnd*)m_close) { ((CXWnd*)this)->Show(false, false); return 1; }
    }
    return CSidlScreenWnd::WndNotification(pWnd, message, data);
}

void NMSLootWnd::PaintItem(CXWnd* name, CXWnd* icon, const PendingItem* item, CXWnd* gear, CXWnd* quest) {
    CXWnd* selected=!item?nullptr:((item->category&2)?quest:((item->category&4)?gear:name));
    for(auto control:{name,gear,quest}) if(control) {
        if(control==selected) control->SetWindowTextA(CXStr((PCHAR)item->name.c_str()));
        control->Show(control==selected,false);
    }
    if(icon) {
        // RoF2 CButtonWnd::Draw (0x896cf0) uses six borrowed decal animations
        // at 0x240..0x254; its destructor never frees these manager-owned assets.
        // Guard the native class before touching its build-specific layout.
        const uintptr_t draw=(*reinterpret_cast<uintptr_t**>(icon))[3];
        const uintptr_t base=reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
        if(draw!=base+0x496cf0) {icon->Show(false,false);return;}
        CTextureAnimation* animation=nullptr;
        if(item && pSidlMgr) {
            char id[64];std::snprintf(id,sizeof(id),"NMSLoot_Icon_%u",item->icon);
            animation=pSidlMgr->FindAnimation(CXStr(id));
        }
        auto decals=reinterpret_cast<CTextureAnimation**>(reinterpret_cast<uint8_t*>(icon)+0x240);
        for(int i=0;i<6;++i) decals[i]=animation;
        icon->Show(item && animation,false);
    }
}

void NMSLootWnd::UpdateMenus(CComboWnd* solo,CComboWnd* group,CXWnd* voted,const PendingItem* item) {
    const bool isGroup=item && item->group_token;
    const bool done=isGroup && (item->group_flags==0xa000 || item->group_flags==0xc000);
    if(solo) {solo->SetChoice(0);((CXWnd*)solo)->Show(item && !isGroup,false);}
    if(group) {group->SetChoice(0);((CXWnd*)group)->Show(isGroup && !done,false);}
    if(voted) voted->Show(done,false);
}

void NMSLootWnd::ChooseMenu(size_t index,CComboWnd* menu,bool group) {
    if(!menu || index>=m_pending.size() || (m_pending[index].group_token!=0)!=group) return;
    const int choice=menu->GetCurChoice();
    // RoF2 CComboWnd forwards XWM_LCLICK only after the list selection closes.
    // The first entry is a prompt, never an action. Selection applies immediately.
    if(group) {
        if(choice<1 || choice>4) return;
        const uint8_t actions[]={1,2,4,0};
        Decide(index,actions[choice-1],nullptr);
    } else {
        const auto decision=LootChoices::Solo(choice);
        if(!decision.action) return;
        Decide(index,decision.action,decision.rule);
    }
}

void NMSLootWnd::InstallScrollbar() {
    if(!m_scrollbar) return;
    auto original=((PCSIDLWND)m_scrollbar)->pvfTable;
    const uintptr_t base=reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
    // Verified RoF2 native OnVScroll / SetVScrollPos. Only this owned child gets
    // a private vtable; restore it before the parent destroys its children.
    if(reinterpret_cast<uintptr_t>(original->OnVScroll)!=base+0x467730 ||
       reinterpret_cast<uintptr_t>(original->SetVScrollPos)!=base+0x4678f0) return;
    m_scroll_original=original;
    m_scroll_table=new CSIDLWNDVFTABLE(*original);
    m_scroll_table->OnVScroll=reinterpret_cast<void*>(&NMSLootWnd::ScrollNotification);
    ((PCSIDLWND)m_scrollbar)->pvfTable=m_scroll_table;
}

int __fastcall NMSLootWnd::ScrollNotification(CXWnd* bar,void*,unsigned int code,int position) {
    auto w=s_instance;
    if(!w || bar!=w->m_scrollbar) return 0;
    const int maximum=w->m_pending.size()>LootVisibleRows?(int)(w->m_pending.size()-LootVisibleRows):0;
    int next=(int)w->m_first_item;
    // Native RoF2 codes: up/repeat 0/1, down/repeat 2/3, thumb 4, page 5/6.
    // Scroll in whole items; pixel range keeps thumb proportions at five rows.
    switch(code) {
        case 0:case 1:--next;break;
        case 2:case 3:++next;break;
        case 4:next=(std::max)(0,position)/48;break;
        case 5:next-=(int)LootVisibleRows;break;
        case 6:next+=(int)LootVisibleRows;break;
        default:return 0;
    }
    next=(std::max)(0,(std::min)(next,maximum));
    if((size_t)next!=w->m_first_item) {w->m_first_item=(size_t)next;w->RefreshUI();}
    return 0;
}
