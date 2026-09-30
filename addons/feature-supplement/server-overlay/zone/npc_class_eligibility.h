#pragma once
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <limits>
namespace NpcClassEligibility {
inline std::unordered_set<uint32_t> Parse(std::istream& input) {
 std::unordered_set<uint32_t> result; std::string line;
 while (std::getline(input,line)) {
  line=line.substr(0,line.find('#')); std::istringstream row(line);
  uint64_t id=0; std::string extra;
  if ((row>>id) && !(row>>extra) && id>0 && id<=std::numeric_limits<uint32_t>::max()) result.insert(static_cast<uint32_t>(id));
 }
 return result;
}
inline bool Utility(unsigned body, bool untargetable) {
 return untargetable || body==11 || body==33 || body==60 || body>=65;
}
inline bool Excluded(uint32_t id, unsigned body, bool untargetable) {
 if (Utility(body,untargetable)) return true;
 static const auto ids=[](){std::ifstream file("npc_multiclass_exclusions.txt");return Parse(file);}();
 return ids.count(id)!=0;
}
}
