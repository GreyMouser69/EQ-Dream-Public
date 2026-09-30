#pragma once
// One equipped form, once per character. No bonuses from bags or bank.
namespace Winterheart {
inline unsigned Form(Mob* m) {
 if (!m || !m->IsClient() || !(m->CastToClient()->GetClassesBits() & 15906)) return 0;
 auto* c=m->CastToClient();
 if(c->GetItemIDAt(13)==900601) return 1;
 if(c->GetItemIDAt(14)==900602) return 2;
 if(c->GetItemIDAt(11)==900603) return 3;
 return 0;
}
inline bool Spell(Mob* m,uint16 id) {
 if(!Form(m) || !IsValidSpell(id) || IsCombatSkill(id) ||
    m->GetEntityVariable("ProcHint")=="true" || m->IsCombatProc(id)) return false;
 for(int i=0;i<16;++i) if((15906 & (1<<i)) && spells[id].classes[i]<254) return true;
 return false;
}
inline int64 Damage(Mob* m,uint16 id,int64 value) {
 if(!Spell(m,id) || IsBeneficialSpell(id)) return value;
 const int pct=Form(m)==1?25:Form(m)==2?10:15;
 return value+value*pct/100;
}
}
