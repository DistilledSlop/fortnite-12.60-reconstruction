// Uncounted source draft, not a build input; all calls/layouts below remain open.
#include <cstdint>
namespace inventory_create80 {
using P = std::uintptr_t;
extern P ptr(P, unsigned);
extern std::int32_t i32(P, unsigned);
extern void put_ptr(P, unsigned, P);
extern void put_u16(P, unsigned, std::uint16_t);
extern P item_definition_1407c1e20(P entry);
extern P class_142b5be40();
extern P class_142b5bd80();
extern P v518(P definition, P uninitialized_output_storage);
extern P construct_1406828e0(P inventory, P klass, int, int, int, int, int);
extern P fallback_142040900(P inventory);
extern void entry_copy_142025400(P destination, P source);
extern void parent_142038ae0(P entry, P inventory, int zero);
extern P capture_vtable(P object);
extern std::uint32_t value_14202d340(P source_entry);
extern void captured_v4c8(P table, P object, std::uint32_t value);

static bool class_chain(P object_class, P required) {
    const auto index = i32(required, 0x38);
    if (index > i32(object_class, 0x38)) return false;
    const P table = ptr(object_class, 0x30);
    // Signed hierarchy index is part of the native table address computation.
    return ptr(table + static_cast<std::intptr_t>(index) * 8, 0) == required + 0x30;
}

// 14205f090: derive a class from item definition or use the fallback allocator,
// attach/copy the entry, set two adjacent bytes and invoke initialization.
P create(P inventory, P entry) {
    P object = 0;
    const P definition = item_definition_1407c1e20(entry);
    if (definition) {
        const P definition_class = class_142b5be40();
        if (class_chain(ptr(definition, 0x10), definition_class)) {
            alignas(8) std::uint8_t output_storage[8]; // source has no pre-zeroing
            const P class_result = v518(definition, reinterpret_cast<P>(output_storage));
            if (ptr(class_result, 0) != 0) {
                const P required = class_142b5bd80();
                if (required) {
                    const P selected = ptr(class_result, 0); // reload after class call
                    // The original null test follows the hierarchy reads.
                    if (class_chain(selected, required) && selected != 0)
                        object = construct_1406828e0(inventory, selected, 0, 0, 0, 0, 0);
                    else object = fallback_142040900(inventory);
                } else object = fallback_142040900(inventory);
            } else object = fallback_142040900(inventory);
        } else object = fallback_142040900(inventory);
    } else object = fallback_142040900(inventory);
    // A null result from the selected-class allocator does not retry fallback.
    if (object) {
        put_ptr(object, 0xd8, inventory);
        entry_copy_142025400(object + 0xe0, entry);
        parent_142038ae0(object + 0xe0, inventory, 0);
        const P table = capture_vtable(object);
        put_u16(object, 0xd0, 0x100);
        const auto value = value_14202d340(entry);
        captured_v4c8(table, object, value);
    }
    return object;
}
}
