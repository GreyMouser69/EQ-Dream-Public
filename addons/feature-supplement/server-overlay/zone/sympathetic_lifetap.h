#pragma once
#include <cstdint>

namespace EQDreamSympatheticLifetap {
// Verified Prismatic Strike I-X and XI-XV spell families on NMS.
constexpr bool IsStrike(uint32_t spell) {
    return (spell >= 23730 && spell <= 23739) || (spell >= 33233 && spell <= 33237);
}

// SpellFinished resolves synchronously. Keep the originating lifetap attached
// only to this proc call, restoring the enclosing context after nested procs.
class Scope {
public:
    Scope(const void* caster, const void* target, uint32_t spell, bool lifetap)
        : previous(current), caster(caster), target(target), spell(spell),
          enabled(lifetap && IsStrike(spell) && caster && target && caster != target) {
        current = this;
    }
    ~Scope() { current = previous; }
    Scope(const Scope&) = delete;
    Scope& operator=(const Scope&) = delete;

    static bool Claim(const void* caster, const void* target, uint32_t spell, int64_t damage) {
        if (!current || !current->enabled || current->caster != caster ||
            current->target != target || current->spell != spell) return false;
        // Consume before healing: healing hooks cannot claim this damage again.
        current->enabled = false;
        return damage > 0;
    }
private:
    inline static thread_local Scope* current = nullptr;
    Scope* previous;
    const void* caster;
    const void* target;
    uint32_t spell;
    bool enabled;
};
}
