// UNCOUNTED native behavioral drafts. No adapter supplies fallback effects.
#include <cstdint>
#include <cstring>
namespace pawn_poi74 {
using Byte = std::uint8_t;
using Word = std::uint64_t;
struct Object;
struct Iterator;
template<class T> T& At(void* p, unsigned offset) {
    return *reinterpret_cast<T*>(static_cast<Byte*>(p) + offset);
}
extern void* Class_1427a1790();
extern void* Class_142a3d9f0();
extern bool IsA_OriginalClassIndexTable(Object*, void*);
extern const char16_t* Separator_1459a98c4();
extern const char16_t* FindSubstring(const char16_t*, const char16_t*);
extern Word MakeName_142cd4ab0(const char16_t*, int);
extern void RemoveDynamic_1406d80a0(void* delegate, Object*, Word name);
extern void* VirtualAf0(Object*);
extern void* Virtual150(Object*);
extern Iterator* UninitializedIteratorStorage();
extern void IteratorInit_14468fb70(void* world, Iterator*);
extern std::int32_t& IteratorIndex(Iterator*);
extern std::int32_t IteratorLiveCount(Iterator*);
extern void* IteratorWeakAddress(Iterator*, std::int32_t);
extern Object* WeakGet(void*);
extern void* PoiLookup_141ed6bb0(void* manager, Word arg2, Object*);
extern void PoiSetText_141edc620(void*, Word arg3, Byte arg4);

// Native142301790 uses only self and the second raw argument. The exec's
// reflected labels and eight stepped arguments do not establish its ABI.
// In particular arg2 is used as an object here, despite the exec's Momentum label.
void AITargetDied(Object* self, Object* arg2) {
    if (At<Byte>(self, 0xf0) != 3 || !arg2) return;
    void* type = Class_1427a1790();
    if (!IsA_OriginalClassIndexTable(arg2, type) || At<Byte>(self, 0xf0) != 3) return;
    auto* first = At<Object**>(self, 0x13f8);
    // Initial data pointer is retained; the count is read again on each compare.
    for (auto* cursor = first; cursor != first + At<std::int32_t>(self, 0x1400); ++cursor) {
        if (*cursor == arg2) {
            auto index = static_cast<std::int32_t>(cursor - first);
            if (index != -1) {
                auto tail = At<std::int32_t>(self, 0x1400) - index - 1;
                std::memmove(first + index, first + index + 1,
                             static_cast<std::int64_t>(tail) * 8);
                --At<std::int32_t>(self, 0x1400);
            }
            break;
        }
    }
    const char16_t* last = nullptr;
    const char16_t* match = FindSubstring(u"&AFortPlayerPawn::AITargetDied", Separator_1459a98c4());
    while (match) {
        last = match;
        match = FindSubstring(match + 1, Separator_1459a98c4()); // native +2 bytes
    }
    // Preserve the recovered +4-byte address even for absent separator; its
    // literal/encoding contract must be resolved before executable promotion.
    auto* suffix = reinterpret_cast<const char16_t*>(reinterpret_cast<std::uintptr_t>(last) + 4);
    Word name = MakeName_142cd4ab0(suffix, 1);
    RemoveDynamic_1406d80a0(static_cast<Byte*>(static_cast<void*>(arg2)) + 0x928, self, name);
}

// Native1419aeb90: same POI iterator family as GoldenPoiLocationTags, with a
// per-object text update. Invalid entries still reach lookup as null objects.
void SetLocationText(Object* self, Word arg2, Word arg3, Byte arg4) {
    void* manager = VirtualAf0(self);
    if (!manager) return;
    void* world = Virtual150(self);
    Iterator* iterator = UninitializedIteratorStorage();
    IteratorInit_14468fb70(world, iterator);
    while (IteratorIndex(iterator) >= 0 && IteratorIndex(iterator) < IteratorLiveCount(iterator)) {
        Object* object = WeakGet(IteratorWeakAddress(iterator, IteratorIndex(iterator)));
        if (object) {
            void* type = Class_142a3d9f0();
            if (!IsA_OriginalClassIndexTable(object, type)) object = nullptr;
        }
        void* poi = PoiLookup_141ed6bb0(manager, arg2, object);
        if (poi) PoiSetText_141edc620(poi, arg3, arg4);
        ++IteratorIndex(iterator);
    }
    // No iterator destruction appears in this recovered body.
}
} // namespace pawn_poi74
