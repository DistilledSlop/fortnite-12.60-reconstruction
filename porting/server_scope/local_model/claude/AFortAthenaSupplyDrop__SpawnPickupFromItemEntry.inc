// CONFIDENCE: HIGH. Original addresses: exec thunk 0x142834fb0, core call 0x140680300.
// The core call is `xor eax,eax ; ret` (asm-verified). It ignores the item entry, count, pawn, position and
// direction, and returns a null pickup. The port keeps that contract: no pickup is spawned.
return nullptr;
