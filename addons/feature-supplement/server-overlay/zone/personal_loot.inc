// Included by corpse.cpp. Personal ownership never becomes public on timeout.
void Corpse::SetPersonalLootOwner(uint32 character_id, uint16 original_id) {
 m_personal_origin_id=original_id;
 m_personal_loot_owner=character_id;
 m_personal_loot_token=static_cast<uint32>(zone->random.Int(1,2147483647));
 m_corpse_delay_timer.Disable();
 SetDecayTimer(1800000);
 for(auto li:m_item_list) li->lootslot=0xffff;
}

void Corpse::SendPersonalLootSnapshot(Client* c, bool automatic) {
 if(!c || !c->Connected() || c->IsZoning() || !IsNPCCorpse() || m_is_locked ||
    !m_personal_loot_owner || !CanPlayerLoot(c->CharacterID())) return;
 if(GetCopper() || GetSilver() || GetGold() || GetPlatinum()) {
  if(c->GetGroup() && c->AutoSplitEnabled()) c->GetGroup()->SplitMoney(GetCopper(),GetSilver(),GetGold(),GetPlatinum(),c);
  else c->AddMoneyToPP(GetCopper(),GetSilver(),GetGold(),GetPlatinum(),true);
  RemoveCash();
 }
 // NMS does not open/close the native corpse window, so the client cannot
 // infer that a corpse became empty after Keep/Sell. Update its visibility.
 if(IsEmpty() && c->ShouldHideCorpse(this)) {
  EQApplicationPacket despawn;
  CreateDespawnPacket(&despawn,false);
  c->QueuePacket(&despawn);
 }
 m_nms_offer_character_id=c->CharacterID();
 m_nms_offered_items.clear();
 if(m_personal_next_slot>EQ::invslot::CORPSE_END && !m_item_list.empty() &&
    std::all_of(m_item_list.begin(),m_item_list.end(),[](const LootItem* li){return li->lootslot==0xffff;})) {
  m_personal_next_slot=EQ::invslot::CORPSE_BEGIN;
  // A fresh generation protects a second page from delayed first-page packets.
  m_personal_loot_token=m_personal_loot_token==2147483647?1:m_personal_loot_token+1;
 }
 const uint32 count=std::min<uint32>(64,m_item_list.size());
 auto out=new EQApplicationPacket(OP_NMSLootOffer,sizeof(NMSLootOfferHeader)+count*sizeof(NMSLootOfferItem));
 auto header=reinterpret_cast<NMSLootOfferHeader*>(out->pBuffer);
 header->corpse_id=GetID();header->character_id=c->CharacterID();
 header->zone_id=zone->GetZoneID();header->instance_id=zone->GetInstanceID();
 header->reserved=m_personal_loot_token;
 strn0cpy(header->looter_name,c->GetCleanName(),sizeof(header->looter_name));
 auto records=reinterpret_cast<NMSLootOfferItem*>(out->pBuffer+sizeof(NMSLootOfferHeader));
 uint32 index=0;
 const auto mask=c->GetInv().GetLookup()->CorpseBitmask;
 for(auto li:m_item_list) {
  if(index>=count) break;
  auto item=database.GetItem(li->item_id);if(!item) continue;
  // Keep slots stable while automatic choices from an earlier snapshot are
  // in flight. Reusing a removed slot could sell the wrong stack of an item.
  if(li->lootslot==0xffff) {
   while(m_personal_next_slot<=EQ::invslot::CORPSE_END && !((uint64(1)<<m_personal_next_slot)&mask)) ++m_personal_next_slot;
   if(m_personal_next_slot>EQ::invslot::CORPSE_END) break;
   li->lootslot=m_personal_next_slot++;
  }
  const uint16 slot=li->lootslot;
  auto &record=records[index++];
  record.item_id=item->ID;record.icon=item->Icon;record.quantity=std::max<int>(1,li->charges);
  record.loot_slot=slot;record.is_bonus_item=0;
  record.flags=0x4000 | (automatic?0:0x2000) | NMSLootCategory(item) | (item->LoreFlag?1:0);
  strn0cpy(record.item_name,item->Name,sizeof(record.item_name));
  strn0cpy(record.source_name,GetCleanName(),sizeof(record.source_name));
  m_nms_offered_items.emplace_back(slot,item->ID);
 }
 header->item_count=index;
 out->size=sizeof(NMSLootOfferHeader)+index*sizeof(NMSLootOfferItem);
 c->QueuePacket(out);safe_delete(out);
}
