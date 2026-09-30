#pragma once
#include <cstdint>
namespace NewbieNpcClasses {
inline bool Suppress(uint32_t zone, unsigned level, uint32_t npc) {
 if (level < 1 || level > 20) return false;
 // Exact custom launch-event roster; do not exempt ordinary named mobs.
 if (npc >= 1120001301u && npc <= 1120001316u) return false;
 switch(zone) {
 case 1:
 case 2:
 case 3:
 case 4:
 case 8:
 case 9:
 case 10:
 case 19:
 case 21:
 case 22:
 case 23:
 case 24:
 case 25:
 case 29:
 case 30:
 case 33:
 case 34:
 case 38:
 case 40:
 case 41:
 case 42:
 case 46:
 case 47:
 case 49:
 case 50:
 case 52:
 case 54:
 case 55:
 case 56:
 case 60:
 case 61:
 case 62:
 case 67:
 case 68:
 case 75:
 case 78:
 case 82:
 case 83:
 case 106:
 case 155:
 case 165:
 return true;
 default: return false;
 }
}
}
