#pragma once
#include <algorithm>
#include <cstdint>
#include <set>
#include <string>
namespace SpellbookPolicy {
constexpr const char* DeletedKey="eqd_spellbook_deleted";
constexpr const char* MinimumKey="eqd_spellbook_minimum";
inline std::set<unsigned> Parse(const std::string& text) {
 std::set<unsigned> out;unsigned n=0;bool valid=false,bad=false;
 for(char c:text+",") {
  if(c==','){if(valid&&!bad&&n>0&&n<65535)out.insert(n);n=0;valid=false;bad=false;}
  else if(c>='0'&&c<='9'){valid=true;if(n>6553)bad=true;else n=n*10+(c-'0');}
  else bad=true;
 }
 return out;
}
inline std::string Encode(const std::set<unsigned>& ids) {
 std::string s;for(auto id:ids){if(!s.empty())s+=',';s+=std::to_string(id);}return s;
}
inline unsigned Minimum(const std::string& s) {
 if(s.empty())return 1;unsigned n=0;
 for(char c:s){if(c<'0'||c>'9'||n>25)return 1;n=n*10+c-'0';}
 return n>=1&&n<=255?n:1;
}
template<class Levels> inline unsigned Level(const Levels& levels,uint32_t bits,unsigned low=1,unsigned high=254) {
 unsigned found=255;
 for(unsigned i=0;i<16;i++) if((bits&(1u<<i))&&levels[i]>=low&&levels[i]<=high&&levels[i]<255)
  found=std::min(found,(unsigned)levels[i]);
 return found;
}
}
