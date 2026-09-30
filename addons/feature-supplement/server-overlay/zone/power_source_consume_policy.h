#pragma once

namespace EQDreamPowerSource {
// Binding is not progression: SSF binds even freshly looted duplicates.
inline bool CanConsume(bool ssf, bool attuned, bool has_progress_record) {
    return !has_progress_record && (ssf || !attuned);
}
constexpr const char* UsedKey = "EQDreamPowerSourceUsed";
}
