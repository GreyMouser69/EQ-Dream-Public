// Included only by corpse.cpp after the existing NMS wire structures.
// Low bits describe appearance; high bits remain the group state protocol.
static uint16 NMSLootCategory(const EQ::ItemData* item) {
 if(!item) return 0;
 if(item->QuestItemFlag) return 0x0002;
 if(item->Slots) return 0x0004;
 return 0;
}
static bool NMSGroupLootEnabled() {
 const char* p=std::getenv("EQDREAM_GROUP_LOOT_TEST");
 return !p || std::string(p)=="1";
}
bool Corpse::NMSGroupEligible(Client* c) {
 return c && c->GetGroup() && c->GetGroup()->GetID()==m_nms_group_id &&
  CanPlayerLoot(c->CharacterID()) && !m_is_locked &&
  DistanceSquared(c->GetPosition(),GetPosition())<=40000.0f;
}
bool Corpse::BeginNMSGroupLoot(Client* c) {
 if(m_nms_group_token) { SendNMSGroupSnapshot(); return true; }
 if(!NMSGroupLootEnabled() || !c->GetGroup() || m_item_list.empty() || m_item_list.size()>64) return false;
 m_nms_group_id=c->GetGroup()->GetID();
 std::vector<uint32> roster;
 for(auto m:c->GetGroup()->members) if(m && m->IsClient() && NMSGroupEligible(m->CastToClient())) roster.push_back(m->CastToClient()->CharacterID());
 if(roster.size()<2) {m_nms_group_id=0;return false;}
 m_nms_group_token=static_cast<uint32>(zone->random.Int(1,2147483647));
 m_nms_group_started=Timer::GetCurrentTime();
 uint16 slot=EQ::invslot::CORPSE_BEGIN;
 auto mask=c->GetInv().GetLookup()->CorpseBitmask;
 for(auto li:m_item_list) {
  while(slot<=EQ::invslot::CORPSE_END && !(((uint64)1<<slot)&mask)) ++slot;
  if(slot>EQ::invslot::CORPSE_END) break;
  if(!database.GetItem(li->item_id)) continue;
  li->lootslot=slot;
  EQDreamGroupLoot::Round round;round.item=li->item_id;
  for(auto id:roster) round.ballots.emplace(id,EQDreamGroupLoot::Ballot{});
  m_nms_group_rounds.emplace(slot++,round);
 }
 for(auto id:roster) if(auto member=entity_list.GetClientByCharID(id)) member->Message(Chat::Yellow,"Group loot: choose Need, Greed, Sell or Pass within 60 seconds. Winnings are delivered automatically. Unanswered rolls count as Pass.");
 SendNMSGroupSnapshot();return true;
}
void Corpse::SendNMSGroupSnapshot() {
 if(m_nms_group_rounds.empty()) return;
 for(auto &member:m_nms_group_rounds.begin()->second.ballots) {
  auto c=entity_list.GetClientByCharID(member.first);
  if(!c) continue;
  auto out=new EQApplicationPacket(OP_NMSLootOffer,sizeof(NMSLootOfferHeader)+m_nms_group_rounds.size()*sizeof(NMSLootOfferItem));
  auto h=reinterpret_cast<NMSLootOfferHeader*>(out->pBuffer);
  h->corpse_id=GetID();h->character_id=c->CharacterID();h->zone_id=zone->GetZoneID();h->instance_id=zone->GetInstanceID();
  h->reserved=m_nms_group_token;h->item_count=m_nms_group_rounds.size();
  strn0cpy(h->looter_name,c->GetCleanName(),sizeof(h->looter_name));
  auto records=reinterpret_cast<NMSLootOfferItem*>(out->pBuffer+sizeof(NMSLootOfferHeader));
  int i=0;
  for(auto &entry:m_nms_group_rounds) {
   auto &round=entry.second;auto &r=records[i++];auto item=database.GetItem(round.item);
   r.loot_slot=entry.first;r.item_id=round.item;r.icon=item?item->Icon:0;r.quantity=1;r.is_bonus_item=0;
   r.flags=0x8000;
   if(round.resolved) r.flags=(!round.delivered && round.winner==c->CharacterID())?0xc000:0x9000;
   else if(round.ballots.at(c->CharacterID()).choice) r.flags=0xa000;
   r.flags |= NMSLootCategory(item);
   strn0cpy(r.item_name,item?item->Name:"Unknown item",sizeof(r.item_name));
   strn0cpy(r.source_name,GetCleanName(),sizeof(r.source_name));
  }
  c->QueuePacket(out);safe_delete(out);
 }
}
void Corpse::ProcessNMSGroupLoot() {
 if(!m_nms_group_token || m_is_locked) return;
 bool changed=false;
 for(auto &entry:m_nms_group_rounds) {
  auto &r=entry.second;
  if(r.resolved) {if(TryDeliverNMSGroupLoot(entry.first)) changed=true;continue;}
  if(!r.Complete() && uint32(Timer::GetCurrentTime()-m_nms_group_started)<60000) continue;
  r.PassUnanswered();
  r.Resolve([this](uint32 id){return NMSGroupEligible(entity_list.GetClientByCharID(id));},[](int n){return zone->random.Int(0,n-1);});
  changed=true;
  auto item=database.GetItem(r.item);auto winner=entity_list.GetClientByCharID(r.winner);
  for(auto &b:r.ballots) if(auto c=entity_list.GetClientByCharID(b.first)) {
   for(auto &v:r.ballots) if(v.second.choice && v.second.choice!=3) {
    auto voter=entity_list.GetClientByCharID(v.first);
    c->Message(Chat::Yellow,"Group loot: %s rolled %d (%s) for %s.",voter?voter->GetCleanName():"Departed player",v.second.roll,v.second.choice==1?"Need":(v.second.choice==2?"Greed":"Sell"),item?item->Name:"item");
   }
   c->Message(Chat::Yellow,"Group loot: %s - %s.",item?item->Name:"item",winner?fmt::format("{} wins; automatic delivery follows",winner->GetCleanName()).c_str():"all passed or departed; item remains for ordinary corpse looting once rolls finish");
  }
  TryDeliverNMSGroupLoot(entry.first);
 }
 if(changed) SendNMSGroupSnapshot();
}
bool Corpse::HandleNMSGroupResponse(Client* c,uint16 slot,uint32 item,uint8 action,uint32 token) {
 if(!m_nms_group_token) return false;
 // Block legacy clients' auto-Keep/Sell and stale/crafted messages.
 if(token!=m_nms_group_token || !NMSGroupEligible(c)) return true;
 auto it=m_nms_group_rounds.find(slot);
 if(it==m_nms_group_rounds.end() || it->second.item!=item) return true;
 // Expire before accepting a packet at/after the absolute deadline.
 if(uint32(Timer::GetCurrentTime()-m_nms_group_started)>=60000) ProcessNMSGroupLoot();
 auto &r=it->second;
 if((action>=10 && action<=12) || action==15) {
  if(r.Vote(c->CharacterID(),action==15?4:action-9,zone->random.Int(1,100))) {ProcessNMSGroupLoot();SendNMSGroupSnapshot();}
 } else if(action==13 && r.resolved && !r.delivered && r.winner==c->CharacterID()) {
  // Compatibility with older clients; normal delivery requires no click.
  if(TryDeliverNMSGroupLoot(slot)) SendNMSGroupSnapshot();
 }

 return true;
}
bool Corpse::NMSGroupLocksSlot(uint16 slot) const {
 auto i=m_nms_group_rounds.find(slot);
 return i!=m_nms_group_rounds.end() && (!i->second.resolved || (i->second.winner && !i->second.delivered));
}

bool Corpse::TryDeliverNMSGroupLoot(uint16 slot) {
 auto it=m_nms_group_rounds.find(slot);
 if(it==m_nms_group_rounds.end()) return false;
 auto &r=it->second;
 if(!r.resolved || !r.winner || r.delivered || m_nms_group_delivering || m_is_locked) return false;
 const uint32 now=Timer::GetCurrentTime();
 if(r.delivery_attempted && uint32(now-r.last_delivery_attempt)<5000) return false;
 auto c=entity_list.GetClientByCharID(r.winner);
 if(!NMSGroupEligible(c)) return false;
 // Never steal a corpse that somebody is currently looting.
 if(m_being_looted_by_entity_id!=0xFFFFFFFF && m_being_looted_by_entity_id!=0 && m_being_looted_by_entity_id!=c->GetID()) return false;
 r.delivery_attempted=true;r.last_delivery_attempt=now;
 auto li=GetItem(slot);
 if(!li || li->item_id!=r.item) return false;
 auto item=database.GetItem(r.item);
 if(item && r.ballots.at(r.winner).choice==4) {
  // Match the existing NMS solo sell value and pay only the roll winner.
  const uint64 quantity=item->Stackable?std::max<int16>(1,li->charges):1;
  const uint64 price=static_cast<uint64>((static_cast<double>(item->Price)*quantity*RuleR(Merchant,BuyCostMod))+0.5);
  r.delivered=true; // Consume exactly once before removing the corpse item.
  RemoveItem(li);
  if(price) c->AddMoneyToPP(price,true);
  c->Message(Chat::MerchantExchange,"Group loot: you won the Sell roll. NMS sold %s for %s.",item->Name,Strings::Money(price/1000,(price/100)%10,(price/10)%10,price%10).c_str());
  LogInfo("NMS Group Loot: sold item [{}] quantity [{}] corpse [{}] for [{}] copper to [{}]",r.item,quantity,GetID(),price,c->GetCleanName());
  return true;
 }
 // Native looting can fall back to the cursor. Avoid repeated cursor/error
 // packets and preserve the award on the corpse until restrictions clear.
 if(!item || !c->GetInv().CursorEmpty() || c->CheckLoreConflict(item) || !c->GetInv().HasSpaceForItem(item, item->Stackable?std::max<int16>(1,li->charges):1)) {
  if(!r.delivery_notified) {c->Message(Chat::Yellow,"Group loot: your prize is reserved on the corpse. Make inventory space and clear any cursor or Lore conflict; delivery will retry automatically while you are nearby.");r.delivery_notified=true;}
  return false;
 }
 m_loot_request_type=LootRequestType::AllowedPVE;m_being_looted_by_entity_id=c->GetID();
 EQApplicationPacket packet(OP_LootItem,sizeof(LootingItem_Struct));
 auto req=reinterpret_cast<LootingItem_Struct*>(packet.pBuffer);
 req->lootee=GetID();req->looter=c->GetID();req->slot_id=slot;req->auto_loot=1;
 m_nms_group_delivering=true;LootCorpseItem(c,&packet,true);m_nms_group_delivering=false;
 ResetLooter();
 if(!GetItem(slot)) {r.delivered=true;c->Message(Chat::Yellow,"Group loot: %s awarded automatically.",item->Name);return true;}
 if(!r.delivery_notified) {c->Message(Chat::Yellow,"Group loot: delivery is pending on the corpse and will retry automatically while you are nearby.");r.delivery_notified=true;}
 return false;
}
