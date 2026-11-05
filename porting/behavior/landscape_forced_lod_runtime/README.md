# Original landscape ForcedLod batch evidence

Program: `FortniteClient-Win64-Shipping.runtime.exe` (12.60 CL 13477524), virtual addresses below. Captures are bounded excerpts from read-only Ghidra decompilation/disassembly; the original project was not saved. The existing full binding capture is `porting/generated/apollo-material-evidence/landscape-vf-element/0x143ae8e90.{asm,c}`.

- `0x143ae8e90`, at `0x143ae92b1`, supplies the address of batch user data `+0x2c` to the parameter write helper. The parameter binder capture `porting/generated/apollo-material-evidence/landscape-vf-virtuals/0x143add1e0.c` associates the matching shader parameter with `ForcedLod`.
- `0x143adc890`, at `0x143adc8d3`, initializes each 0x30-byte element's field at `+0x2c` to `0xffffffff`. `0x143aea870` independently initializes its newly allocated element to the same value at `0x143aea955`, then populates the landscape batch and uses it as mesh user data (see decompilation; the excerpt captures initialization only).
- `0x143ae8090` initializes a local to the result of `0x140693f70` when the tested view flag is set, or to zero otherwise. At `0x143ae8451`, the local is copied into the batch's `+0x2c`; the same element separately receives its current LOD at `+0x28`. `0x140693f70` returns `0xffffffff` in this shipping executable. The flag's identification as the LOD show flag follows the matching engine control flow; the assembly alone establishes only the tested bit.

These excerpts establish raw field values and the captured producers, not all possible producers or the shader declaration's type. The modern UE 4.25 landscape batching and view handling differ, so a binding to a constant or to `CurrentLOD` is not established as parity. No runtime code was changed on this evidence.
