#pragma once
#include <cstddef>

// Portable decision core for 0x14081d340. No UObject or brush construction.
namespace CommonActionKeyLookupCore {
// Adapter contract: SlotCount(), ResolveSlot(i), InputType(controller),
// GamepadType(controller), MapCount(controller), MapKey(controller,i),
// MapBrush(controller,i), SetCount(controller), SetKeyCount(controller,i),
// SetKey(controller,i,j), SetBrush(controller,i), NameIndex(key), NameNumber(key).
// ResolveSlot must skip null objects and perform the original conditional lazy
// resolution/reload. Brush accessors return the same nullable pointer type;
// matching brush pointers must be non-null and remain owned by the adapter.
// The caller performs brush copy
// and fallback. Keys supports size(), operator[], and range iteration. Name accessors return the
// two 32-bit FName components, not display strings or whole FKey equality.
template<class Adapter, class Keys>
auto Find(Adapter& a, const Keys& keys, unsigned char input, unsigned char gamepad)
    -> decltype(a.MapBrush(a.ResolveSlot(std::size_t{}), std::size_t{}))
{
    using Result = decltype(a.MapBrush(a.ResolveSlot(std::size_t{}), std::size_t{}));
    auto same = [&](const auto& x, const auto& y) {
        return a.NameIndex(x) == a.NameIndex(y) && a.NameNumber(x) == a.NameNumber(y);
    };
    for (std::size_t i = 0; i < a.SlotCount(); ++i) {
        auto controller = a.ResolveSlot(i);
        if (!controller || a.InputType(controller) != input) continue;
        if (input == 1 && a.GamepadType(controller) != gamepad) continue;
        if (keys.size() == 0) return Result{};
        if (keys.size() == 1) {
            for (std::size_t j = 0; j < a.MapCount(controller); ++j)
                if (same(a.MapKey(controller, j), keys[0])) return a.MapBrush(controller, j);
            return Result{};
        }
        for (std::size_t j = 0; j < a.SetCount(controller); ++j) {
            const auto count = a.SetKeyCount(controller, j);
            if (count < 2 || count != keys.size()) continue;
            bool all = true;
            for (const auto& key : keys) {
                bool found = false;
                for (std::size_t k = 0; k < count; ++k)
                    if (same(a.SetKey(controller, j, k), key)) { found = true; break; }
                if (!found) { all = false; break; }
            }
            if (all) return a.SetBrush(controller, j);
        }
        return Result{};
    }
    return Result{};
}
}
