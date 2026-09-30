#pragma once

#include "MQ2Main.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class NMSLootWnd : public CCustomWnd {
public:
    NMSLootWnd();
    ~NMSLootWnd();

    int WndNotification(CXWnd* pWnd, unsigned int message, void* data);

    static void Initialize();
    static void Shutdown();
    static void OnIncomingPacket(uint16_t opcode, void* buffer, size_t size);
    static void ToggleWindow();

private:
    struct PendingItem {
        uint16_t corpse_id;
        uint32_t loot_slot;
        uint32_t item_id;
        uint32_t icon;
        uint32_t personal_token = 0;
        uint32_t group_token = 0;
        uint16_t group_flags = 0;
        bool remember = false;
        uint16_t category = 0;
        std::string name;
    };

    static NMSLootWnd* s_instance;
    void RefreshUI();
    void ChooseMenu(size_t index, CComboWnd* menu, bool group);
    static void UpdateMenus(CComboWnd* solo, CComboWnd* group, CXWnd* voted, const PendingItem* item);
    CComboWnd* m_solo_menu[12]={};
    CComboWnd* m_group_menu[12]={};
    CXWnd* m_voted[12]={};
    static void PaintItem(CXWnd* name, CXWnd* icon, const PendingItem* item, CXWnd* gear, CXWnd* quest);
    bool m_had_pending=false;
    size_t m_first_item=0;
    CXWnd *m_scrollbar=nullptr,*m_item_count=nullptr;
    PCSIDLWNDVFTABLE m_scroll_original=nullptr;
    PCSIDLWNDVFTABLE m_scroll_table=nullptr;
    static int __fastcall ScrollNotification(CXWnd* bar,void*,unsigned int code,int position);
    void InstallScrollbar();
    void Decide(size_t index, uint8_t action, const char* rule);
    void DecideAll(uint8_t action, const char* rule);
    void SendDecision(const PendingItem& item, uint8_t action);
    std::string RuleFor(const PendingItem& item) const;
    void SaveRule(const PendingItem& item, const char* rule) const;
    std::string IniPath() const;
    void RefreshRules();
    bool EnsureOnScreen();
    void ChangeRule(const char* action);
    void AddRuleFilter(const char* action);
    std::string RuleInputText() const;
    struct SavedRule { std::string section, key, value; };
    std::vector<SavedRule> m_rules;
    CListWnd* m_rule_list = nullptr;
    CXWnd* m_rule_input = nullptr;
    CXWnd* m_rule_add = nullptr;
    CXWnd* m_rule_delete = nullptr;
    CXWnd* m_rule_actions[6] = {};
    std::string m_filter_action = "Keep";

    CComboWnd* m_rule_category = nullptr;
    CXWnd* m_rule_count = nullptr;
    int m_rule_category_choice = 0;
    std::string m_rule_search;
    CXWnd* m_search_input = nullptr;
    CXWnd* m_search_go = nullptr;
    CXWnd* m_search_clear = nullptr;
    void SearchRules(bool clear);
    void ToggleAuto(size_t index);
    std::string GroupRuleFor(const PendingItem& item) const;
    CXWnd* m_auto[12] = {};
    CXWnd* m_group_actions[4] = {};
    CXWnd* m_names[12];
    CXWnd* m_gear_names[12]={};
    CXWnd* m_quest_names[12]={};
    CXWnd* m_slots[12];
    CButtonWnd* m_keep_all;
    CButtonWnd* m_sell_all;
    CButtonWnd* m_loot_all;
    CButtonWnd* m_close;
    std::vector<PendingItem> m_pending;
};

void CmdNMSLoot(PSPAWNINFO character, PCHAR arguments);
