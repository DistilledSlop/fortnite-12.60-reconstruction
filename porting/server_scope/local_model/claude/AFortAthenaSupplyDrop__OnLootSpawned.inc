// CONFIDENCE: LOW. Original addresses: exec thunk 0x1428347d0; native body not recoverable from captures.
// The thunk only advances the bytecode cursor (P_FINISH). Control then enters a virtualized stub:
// 0x14191d000 -> 0x14998573a (push/xchg return trampoline) -> 0x148ec417a / 0x1490beeb2 / 0x14923284d
// (full register-save entry for a protected handler). Neither the decompiler nor objdump can follow it,
// so no native side effect is recovered. Ported as an empty native body; the loot-spawned reaction lives
// on the Blueprint side.
