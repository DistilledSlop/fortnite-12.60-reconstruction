// Rough source lift of 0x141a5f1c0. ABI and object layouts remain unresolved.
#include <cstddef>
#include <cstdint>

namespace large_predicate80 {
using byte = std::uint8_t;

// These declarations describe individual observed native calls, not implementations.
extern std::uintptr_t call_141e5b2a0();
extern void call_141e5f200(std::uintptr_t, void*, char, byte*);
extern const byte* call_141ed1800();
extern char call_142cd8ed0(std::uint32_t, int);
// Return is the observed AL, including the extraout_AL cases at +0x448.
extern char call_14192b2d0(void* array_header, const byte* value);

// Layout reads are explicit adapters until alignment, aliasing and ABI are resolved.
extern std::int32_t read_i32(const void* address);
extern std::uint32_t read_u32(const void* address);
extern const byte* read_pointer(const void* address);

struct Case { std::uint16_t offset; byte result; };

// Inline equality loops in the first part of the native body. The high word is
// captured after the predicate call; the low word is re-read for every element.
static bool inline_match(byte* input, std::size_t offset) {
    const byte* source = call_141ed1800();
    const char empty = call_142cd8ed0(read_u32(source + offset), 0);
    const std::int32_t high = read_i32(source + offset + 4);
    if (high == 0 && empty != 0) return false;
    for (std::size_t header_offset = 0; header_offset != 32; header_offset += 16) {
        const byte* p = read_pointer(input + header_offset);
        const std::int32_t count = read_i32(input + header_offset + 8);
        const byte* end = p + static_cast<std::int64_t>(count) * 8;
        while (p != end) {
            if (read_i32(p) == read_i32(source + offset) && read_i32(p + 4) == high)
                return true;
            p += 8;
        }
    }
    return false;
}

// Later comparisons use the actual separate helper, not the inlined loop above.
static bool helper_match(byte* input, std::size_t offset) {
    const byte* source = call_141ed1800();
    const char empty = call_142cd8ed0(read_u32(source + offset), 0);
    if (read_i32(source + offset + 4) == 0 && empty != 0) return false;
    return call_14192b2d0(input, source + offset) != 0 ||
           call_14192b2d0(input + 16, source + offset) != 0;
}

byte function_141a5f1c0(byte* input, char param2) {
    byte result[16]; // Only byte zero is initialized by this body.
    result[0] = 0x32;
    const auto context = call_141e5b2a0();
    call_141e5f200(context, input, param2, result);
    if (result[0] != 0x32) return result[0];

    static constexpr Case first[] = {
        {0x1628,0x18}, {0x1638,0x19}, {0x1640,0x1a}, {0x1648,0x1b},
        {0x0b00,0x1c}, {0x03b0,0x17}, {0x03b8,0x24}, {0x03c0,0x26},
        {0x0420,0x20}, {0x0428,0x21}, {0x0418,0x1f}
    };
    for (const Case& c : first) if (inline_match(input, c.offset)) return c.result;
    if (inline_match(input, 0x358)) return static_cast<byte>(param2 * 16 + 1);
    if (inline_match(input, 0x360)) return param2 != 0 ? 0x11 : 0;
    static constexpr Case middle[] = {
        {0x0b20,0x14}, {0x0b30,0x15}, {0x0b38,0x16}, {0x0b28,0x09},
        {0x09e0,0x08}, {0x0a00,0x02}, {0x0aa0,0x05}, {0x0a30,0x03},
        {0x0a50,0x04}
    };
    for (const Case& c : middle) if (inline_match(input, c.offset)) return c.result;
    if (inline_match(input, 0x0a80)) return inline_match(input, 0x0638) ? 6 : 7;
    static constexpr Case last_inline[] = {
        {0x0ad8,0x0a}, {0x0ae0,0x0b}, {0x0ae8,0x0c}, {0x0af0,0x0d},
        {0x0aa8,0x0f}
    };
    for (const Case& c : last_inline) if (inline_match(input, c.offset)) return c.result;
    if (helper_match(input, 0x0ab8)) return 0x0e;
    static constexpr Case last_helper[] = {
        {0x0ac0,0x1d}, {0x0ac8,0x22}, {0x0ad0,0x23}, {0x05a8,0x10},
        {0x0b40,0x1e}, {0x0380,0x30}, {0x0450,0x31}, {0x0ef0,0x28},
        {0x0ef8,0x29}, {0x0f00,0x2a}, {0x0f10,0x2b}, {0x0f18,0x2c},
        {0x0f20,0x2d}, {0x02d0,0x27}, {0x0368,0x11}
    };
    for (const Case& c : last_helper) if (helper_match(input, c.offset)) return c.result;
    return helper_match(input, 0x0448) ? 0x2e : 0x32;
}
} // namespace large_predicate80
