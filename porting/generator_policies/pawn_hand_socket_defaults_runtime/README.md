# Pawn hand socket constructor defaults

The original `AFortPawn` constructor begins at `0x1422fce30`. It zeroes
`WeaponHandSocketName` (`+0x5b8`) and `LeftHandWeaponHandSocketName` (`+0x5c0`)
at `0x1422fcfcd` and `0x1422fcfd4`. The constructor initializes an FName from
the literal `RightHand` at `0x1422fd724..0x1422fd732` and stores it at `+0x5b8`
at `0x1422fd753..0x1422fd75a`. There is no later write to `+0x5c0` in that
constructor, so the left default is `None`. The cached original disassembly and
decompilation are `porting/generated/native111-constructors/0x1422fce30.asm`
and `.c`.

The policy inserts both defaults into the existing generated native constructor,
after native default-subobject finalization. It requires the original parent,
exactly one declaration of each FName field, and one compatible constructor;
existing field assignments fail generation for explicit review. It does not add
an equip-time fallback or change weapon-specific overrides. Blueprint defaults
may still override these inherited native defaults.

Generator integration: import this module and call `finalize(model, header, cpp)`
after `native_default_subobjects.finalize`. Validate generated code and staged
server behavior independently.
