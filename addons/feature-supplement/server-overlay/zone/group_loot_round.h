#pragma once
#include <cstdint>
#include <map>
#include <vector>
#include <algorithm>
namespace EQDreamGroupLoot {
struct Ballot { uint8_t choice = 0; int roll = 0; };
struct Round {
 uint32_t item = 0, winner = 0;
 bool resolved = false, delivered = false;
 bool delivery_attempted=false, delivery_notified=false;
 uint32_t last_delivery_attempt=0;
 std::map<uint32_t, Ballot> ballots;
 bool Vote(uint32_t id, uint8_t choice, int roll) {
  auto i=ballots.find(id);
  if(resolved || i==ballots.end() || i->second.choice || choice<1 || choice>4 || roll<1 || roll>100) return false;
  i->second={choice,choice==3?0:roll}; return true;
 }
 void PassUnanswered() { for(auto &b:ballots) if(!b.second.choice) b.second={3,0}; }
 bool Complete() const { for(auto &b:ballots) if(!b.second.choice) return false; return true; }
 template<class Eligible, class Tie> void Resolve(Eligible eligible, Tie tie) {
  if(resolved) return;
  std::vector<uint32_t> best; int priority=0,high=0;
  for(auto &b:ballots) {
   auto v=b.second;
   if(!eligible(b.first) || v.choice==0 || v.choice==3) continue;
   int p=v.choice==1?3:(v.choice==2?2:1);
   if(p>priority || (p==priority && v.roll>high)) {best.clear();priority=p;high=v.roll;}
   if(p==priority && v.roll==high) best.push_back(b.first);
  }
  winner=best.empty()?0:best.at(tie(static_cast<int>(best.size())));
  resolved=true;
 }
};
}
