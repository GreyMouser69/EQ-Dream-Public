#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
namespace ChaseCycle {
inline uint32_t Next(uint32_t id) {
    if (id >= 900102 && id <= 900106) return id == 900106 ? 900102 : id + 1;
    if (id >= 900122 && id <= 900126) return id == 900126 ? 900122 : id + 1;
    if (id >= 900601 && id <= 900603) return id == 900603 ? 900601 : id + 1;
    return 0;
}
inline bool Enabled() {
    const char* value=std::getenv("EQDREAM_CHASE_CYCLE_MODULE");
    return value && std::strcmp(value,"1")==0;
}
inline bool Ready(int slot, int first, int last, bool fighting, bool casting, int64_t hp) {
    return slot>=first && slot<=last && !fighting && !casting && hp>0;
}
}
