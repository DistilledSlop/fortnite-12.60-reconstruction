// Uncounted native control-flow lifts. ABI, layouts, and ownership remain open.
#include <cstdint>
using Word = std::uintptr_t;
using Byte = std::uint8_t;
using I32 = std::int32_t;

namespace broad_state79 {
// Every memory access and call below is a separate unresolved ABI/layout adapter.
Word read_ptr(Word base, Word offset);
I32 read_i32(Word base, Word offset);
Word global_148351eb8();
Word call_14463f750(Word global, Word first);
Word call_14447f220(Word object, Word first);
Word call_142a3d9f0();
Byte virtual_28(Word embedded_receiver);
Word call_141e41510(Word embedded_receiver, Byte flag);
void call_142c0c240(Word output);
void call_141fc0980(Word object, Word output, Word third);
Word AFortGameStateAthena_StaticClass();
// Native call passes RCX, RDX, R8 unchanged and sets only R9B to 1.
void virtual_1fc8(Word receiver, Word second, Word third, Word incoming_r9_with_low_byte_one);

Byte native_14241bc10(Word first, Word second) {
    if (!second) return 0;
    Word object = call_14463f750(global_148351eb8(), first);
    if (!object) return 0;
    object = call_14447f220(object, first);
    if (!object) return 0;
    const Word klass = call_142a3d9f0();
    const Word object_class = read_ptr(object, 0x10);
    const I32 index = read_i32(klass, 0x38);
    if (index > read_i32(object_class, 0x38)) return 0;
    const Word table = read_ptr(object_class, 0x30);
    if (read_ptr(table, static_cast<Word>(static_cast<std::intptr_t>(index) * 8)) != klass + 0x30)
        return 0;
    const Byte a = virtual_28(object + 0x590);
    const Byte b = virtual_28(second + 0x318);
    return a == b;
}

// Forwarding wrapper; return is the original second argument regardless of callee result.
Word native_1423d5330(Word receiver, Word second, Word third, Word incoming_r9) {
    virtual_1fc8(receiver, second, third, (incoming_r9 & ~Word{0xff}) | 1);
    return second;
}

Word native_141e5f780(Word receiver, Word output, Word third) {
    Word object = call_141e41510(receiver + 0x4548, 1);
    if (!object) call_142c0c240(output);
    else call_141fc0980(object, output, third);
    return output;
}

// Only AL is the boolean result; upper RAX bits reflect the executed path.
Word native_141a2d0f0(Word object) {
    const Word klass = AFortGameStateAthena_StaticClass();
    const Word object_class = read_ptr(object, 0x10);
    const I32 index = read_i32(klass, 0x38);
    Word result = static_cast<Word>(static_cast<std::intptr_t>(index));
    if (index <= read_i32(object_class, 0x38)) {
        result = read_ptr(object_class, 0x30);
        if (read_ptr(result, static_cast<Word>(static_cast<std::intptr_t>(index) * 8)) == klass + 0x30)
            return (result & ~Word{0xff}) | 1;
    }
    return result & ~Word{0xff};
}
}
