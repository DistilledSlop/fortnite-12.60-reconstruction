// Uncounted source recovery; not a build input. Every extern remains an explicit
// native call/layout contract. No fallback implementations or accepted UE ABI.
#include <cstdint>
#include <cstddef>
namespace creative_helpers79 {
using P = std::uintptr_t;
using I = std::int32_t;
using U = std::uint32_t;
using Q = std::uint64_t;
extern P ptr(P, std::size_t);
extern I i32(P, std::size_t);
extern U u32(P, std::size_t);
extern void put_ptr(P, std::size_t, P);
extern void put_i32(P, std::size_t, I);
extern void put_u32(P, std::size_t, U);
extern P runtime_options_default();
extern P empty_string_1459a4b30();
extern I compare_142bcf780(P left, P right);
extern I find_140a38a90(P output_array, P runtime_entry);
extern void grow_array_140690dd0(P array, I previous_count);
extern void allocate_string_140690ac0(P string, I count, I zero);
// The decompiler omitted memcpy's byte count. Keep it open rather than infer
// a count from FString's apparent layout without instruction evidence.
extern void copy_string_memcpy_missing_size(P destination, P source);
extern I log_level_1481348e0();
// Each log adapter preserves the fixed native format, category1481348e4, level2,
// null file/line0, and unresolved variadic register/stack inputs. Return raw RAX.
extern Q log_too_many_description_tags_missing_varargs();
extern Q log_description_tag_too_long_missing_varargs();
extern Q log_publish_lengths_missing_varargs();
extern std::uint8_t text_predicate_142c2e440(P text);
extern P text_value_142c40540(P text);
extern P construct_140682ce0(P storage, P value);
extern P transform_142706020(P runtime, P output, P input, P fourth, P fifth);
extern void atomic_increment_i32(P count);
// Returns pre-decrement low dword (LOCK decrement's original value in source).
extern I atomic_fetch_decrement_i32(P count);
extern void destroy_virtual_0(P control);
extern void delete_virtual_8(P control, I flag);
extern I thread_epoch_248();
extern I guard_148132cd8();
extern P global_148130770();
extern void init_thread_header_145803a44(); // &guard148132cd8
extern void construct_globals_141ebb310(P globals);
extern void register_atexit_1458038a4_145865cb0();
extern void init_thread_footer_1458039e4(); // &guard148132cd8

// 1419d9d90: whitelist/canonicalize input tag strings against 40-byte runtime
// entries. First matching runtime entry wins; output strings are unique.
P canonical_tags(P output, P input) {
    put_ptr(output, 0, 0);
    put_ptr(output, 8, 0);
    P tag = ptr(input, 0);
    const P end = tag + static_cast<std::intptr_t>(i32(input, 8)) * 16;
    while (tag != end) {
        const P runtime = runtime_options_default(); // reload once per input
        P entry = ptr(runtime, 0x40);
        const P entries_end = entry + static_cast<std::intptr_t>(i32(runtime, 0x48)) * 40;
        while (entry != entries_end) {
            const P requested = i32(tag, 8) == 0 ? empty_string_1459a4b30() : ptr(tag, 0);
            const P allowed = i32(entry, 8) == 0 ? empty_string_1459a4b30() : ptr(entry, 0);
            if (compare_142bcf780(allowed, requested) == 0) {
                if (find_140a38a90(output, entry) == -1) {
                    const I index = i32(output, 8);
                    const I count = static_cast<I>(static_cast<U>(index) + 1U);
                    put_i32(output, 8, count);
                    if (i32(output, 12) < count) grow_array_140690dd0(output, index);
                    const P result = ptr(output, 0) + static_cast<std::intptr_t>(index) * 16;
                    put_ptr(result, 0, 0);
                    const I length = i32(entry, 8);
                    const P data = ptr(entry, 0);
                    put_i32(result, 8, length);
                    if (length == 0) put_i32(result, 12, 0);
                    else {
                        allocate_string_140690ac0(result, length, 0);
                        copy_string_memcpy_missing_size(ptr(result, 0), data);
                    }
                }
                break;
            }
            entry += 40;
        }
        tag += 16;
    }
    return output;
}

// 1419ddfd0: only AL is the validity result; keep observed upper RAX provenance.
Q valid_publish_options(P options) {
    const P runtime = runtime_options_default();
    Q result = u32(runtime, 0x38);
    if (static_cast<I>(u32(runtime, 0x38)) < i32(options, 0x38)) {
        if (log_level_1481348e0() > 1) {
            runtime_options_default();
            return log_too_many_description_tags_missing_varargs() & ~Q{0xff};
        }
        return result & ~Q{0xff};
    }
    P tag = ptr(options, 0x30);
    const P end = tag + static_cast<std::intptr_t>(i32(options, 0x38)) * 16;
    for (; tag != end; tag += 16) {
        result = static_cast<Q>(static_cast<std::int64_t>(
            static_cast<I>(static_cast<U>(i32(tag, 8)) - 1U)));
        if (i32(tag, 8) == 0) result = 0;
        if (static_cast<I>(result) > 32) {
            if (log_level_1481348e0() > 1)
                return log_description_tag_too_long_missing_varargs() & ~Q{0xff};
            return result & ~Q{0xff};
        }
    }
    const I title_num = i32(options, 8);
    U length = static_cast<U>(title_num) - 1U;
    result = length;
    U title_length = title_num == 0 ? 0 : length;
    bool valid = static_cast<I>(title_length) > 2;
    if (valid) valid = title_num == 0 || static_cast<I>(length) < 41;
    if (valid && i32(options, 0x18) != 0) {
        length = static_cast<U>(i32(options, 0x18)) - 1U;
        result = length;
        valid = static_cast<I>(length) < 151;
    }
    if (valid && i32(options, 0x50) != 0) {
        length = static_cast<U>(i32(options, 0x50)) - 1U;
        result = length;
        valid = static_cast<I>(length) < 11;
    }
    if (valid) {
        length = u32(options, 0x28);
        result = length;
        if (length == 0) return (result & ~Q{0xff}) | 1;
        result = length - 1U;
        if (static_cast<I>(length - 1U) < 257) return (result & ~Q{0xff}) | 1;
    }
    if (log_level_1481348e0() > 1) result = log_publish_lengths_missing_varargs();
    return result & ~Q{0xff};
}

static void retain(P control) {
    if (control) atomic_increment_i32(control + 8);
}
static void release(P control) {
    if (control && atomic_fetch_decrement_i32(control + 8) == 1) {
        destroy_virtual_0(control);
        if (atomic_fetch_decrement_i32(control + 12) == 1) delete_virtual_8(control, 1);
    }
}

// 142705ec0: temporary lifetimes are disjoint source branches, followed by a
// copied 20-byte value. No output initialization beyond the observed writes.
P transform_text(P runtime, P output, P input, P fourth, P fifth) {
    alignas(8) std::uint8_t string_storage[16];
    alignas(8) std::uint8_t transformed_storage[20];
    alignas(8) std::uint8_t retained_storage[20];
    const bool keep_input = text_predicate_142c2e440(input) != 0;
    P selected;
    P retained_control = 0;
    if (!keep_input) {
        const P value = text_value_142c40540(input);
        const P temporary = construct_140682ce0(reinterpret_cast<P>(string_storage), value);
        selected = transform_142706020(runtime, reinterpret_cast<P>(transformed_storage),
                                      temporary, fourth, fifth);
    } else {
        retained_control = ptr(input, 8);
        const P value = ptr(input, 0);
        retain(retained_control);
        selected = reinterpret_cast<P>(retained_storage);
        put_ptr(selected, 0, value);
        put_ptr(selected, 8, retained_control);
        put_u32(selected, 16, u32(input, 16));
    }
    put_ptr(output, 0, ptr(selected, 0));
    const P control = ptr(selected, 8);
    put_ptr(output, 8, control);
    retain(control);
    put_u32(output, 16, u32(selected, 16));
    if (!keep_input) release(ptr(reinterpret_cast<P>(transformed_storage), 8));
    if (keep_input) release(retained_control);
    return output;
}

// 141ed1800: MSVC guarded singleton, preserving epoch check/constructor/atexit.
P globals() {
    if (thread_epoch_248() < guard_148132cd8()) {
        init_thread_header_145803a44();
        if (guard_148132cd8() == -1) {
            construct_globals_141ebb310(global_148130770());
            register_atexit_1458038a4_145865cb0();
            init_thread_footer_1458039e4();
            return global_148130770();
        }
    }
    return global_148130770();
}
}
