// Uncounted, non-buildable lifts of actual safe-zone dependencies.
// Offsets and calls are evidence, not resolved UE declarations or ABI contracts.
#include <cstdint>
namespace broad_mode79 {
using Byte = std::uint8_t;
using P = void*;
// These accesses require the original object layout and valid lifetime.
extern P pointer_at(P object, unsigned offset);
extern Byte byte_at(P object, unsigned offset);
extern std::int32_t int_at(P object, unsigned offset);
extern const std::int32_t* int_pointer_at(P object, unsigned offset);
extern bool playlist_predicate_141a49590(P playlist); // null behavior belongs to callee
extern bool playlist_predicate_141a4b0d0(P playlist); // null behavior belongs to callee
extern P settings_141ed2350();
extern P settings_141ed1800();
extern bool name_index_is_none_142cd8ed0(std::int32_t index, int zero);

P active_playlist(P state) {
    P playlist = pointer_at(state, 0x1df8);
    if (!playlist) playlist = pointer_at(state, 0x1df0);
    return playlist;
}

Byte fun_14199e430(P state) {
    if (playlist_predicate_141a49590(active_playlist(state))) return 1;
    // Reload: the first call can have effects on the selected playlist.
    if (playlist_predicate_141a4b0d0(active_playlist(state))) return 1;
    return 0;
}

std::uint64_t fun_14199e4e0(P state) {
    P config = settings_141ed2350();
    if (byte_at(config, 0x2c8) == 0) return 0;
    if (playlist_predicate_141a49590(active_playlist(state))) return 0;
    if (playlist_predicate_141a4b0d0(active_playlist(state))) return 0;
    P playlist = active_playlist(state);
    if (playlist) {
        P globals = settings_141ed1800();
        const std::int32_t index = int_at(globals, 0x1780);
        const bool none = name_index_is_none_142cd8ed0(index, 0);
        const std::int32_t number = int_at(globals, 0x1784);
        if (number != 0 || !none) {
            // The native code reloads index from globals during comparisons.
            // A separate accessor preserves that distinction if storage mutates.
            const std::int32_t* entry = int_pointer_at(playlist, 0xd0);
            const std::int32_t* end = entry + static_cast<std::int64_t>(int_at(playlist, 0xd8)) * 2;
            while (entry != end) {
                if (entry[0] == int_at(globals, 0x1780) && entry[1] == number) return 0;
                entry += 2;
            }
            entry = int_pointer_at(playlist, 0xe0);
            end = entry + static_cast<std::int64_t>(int_at(playlist, 0xe8)) * 2;
            while (entry != end) {
                if (entry[0] == int_at(globals, 0x1780) && entry[1] == number) return 0;
                entry += 2;
            }
        }
        if (byte_at(playlist, 0x467) != 0 && byte_at(playlist, 0x467) != 7) return 0;
    }
    return 1;
}
}
