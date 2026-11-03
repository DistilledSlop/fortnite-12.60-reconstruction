# Forced weapon equip: direct call graph, incomplete closure

This is investigation evidence, not a proposed implementation. The input is
`/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
loaded at `0x140000000`. The raw entry windows were disassembled with GNU
`objdump -D -b binary -m i386:x86-64 -M intel --adjust-vma=<VA>`; scratch binary
windows and listings are under `/tmp/gameplay-weapon-equip/`. Captured byte hashes
below identify windows, not semantic or runtime validation.

`0x142314fe0` (196 bytes, SHA-256
`28f916b32325ec4206a1296341a15c749b591a9e4d05f4613f03f57b9f22aa74`)
operates as follows, with all calls retaining their virtual dispatch:

```cpp
// Roles are compared to the raw AActor byte at +0xf0; 3 is Authority.
if (Role == 3 && Weapon != nullptr) {
    auto* inventory = InventoryInterfaceForWeapon(Weapon); // 0x14269c8f0
    auto* item = inventory ? inventory->virtual_b0(Weapon->ItemEntryGuid) : nullptr;
    Weapon->virtual_7b0(item);
}
bool result = Pawn->virtual_c58(Weapon);
if (result) {
    if (Role == 3) {
        if (!Pawn->virtual_6b0())
            Dispatch_142a16c90(Pawn, Pawn->CurrentWeapon);
    } else {
        Dispatch_142a19220(Pawn, Pawn->CurrentWeapon);
    }
}
return result;
```

Reflection identifies `AFortWeapon::ItemEntryGuid` at `+0x8c0` and
`AFortPawn::CurrentWeapon` at `+0x588`. The GUID is copied as all 16 bytes
before the interface call. The authority check is repeated after virtual_c58;
the first check cannot be hoisted. The dispatch receives the *post-call*
CurrentWeapon, which need not equal the incoming weapon. The false result has no
post-dispatch and is returned as the low byte.

`0x14269c8f0` (160 bytes, SHA-256
`2a4c290dadbaec367c4f754cdbdb3b0c57c142d4f47ca36084933c3c692f5a7c`):
call `0x1426a07c0(Weapon)`. If non-null and castable to
`AFortPlayerController` (class getter `0x142a3d9f0`), return the interface
subobject at controller `+0x598`. Otherwise read the weapon's actor owner
at `+0x118` (getter `0x141125940`); if castable to `AFortPawn` (class getter
`0x142a174f0`, whose class literal is `FortPawn`), tail-call its virtual
`+0xbd8`, returning its result. Otherwise return null. The controller resolver
`0x1426a07c0` is substantive and remains unreconstructed; treating it simply as
owner or controller would be unsupported. The interface `+0xb0`, pawn `+0xbd8`,
and weapon `+0x7b0` identity and effects remain unclosed.

`0x142a16c90` and `0x142a19220` are each 61-byte wrappers, SHA-256 respectively
`0163cc708660cf8e8f7e8ef070ddd988b8e61bdcd7a79b774cf1078ecce23ce0` and
`923743c838d3d7b268c47703b70f51f4c62a46ebb84ae48e03d5e79fd202308e`.
Each stores its pointer argument in an 8-byte parameter block, resolves a
function by FName via `0x142ec7730`, then calls the object's virtual `+0x210`
with the resulting UFunction and parameter block (ProcessEvent shape). Their
FName globals are `0x1481ad458` and `0x1481ad538` respectively, containing
captured comparison indices `0x4de07` and `0x4de9d`. Those indices alone do not identify the event names; the original static initializers now close them as `ClientInternalEquipWeapon` and `ServerInternalEquipWeapon`, respectively (see `weapon_event_names_evidence.md`). AFortPawn declares several plausible
one-weapon events, including `OnRep_CurrentWeapon`, `ClientInternalEquipWeapon`,
and `ServerInternalEquipWeapon`; do not substitute one without resolving the
FNames. Wrappers themselves do not directly mutate the pawn, but event dispatch
can have substantial native, Blueprint, and networking effects.

In the captured AFortPlayerPawnAthena vtable at `0x145e2cfd0`, `+0x6b0` is
`0x144430060`, which checks controller `+0x250` and calls its virtual `+0x6a8`,
returning false when absent. `+0xc58` is `0x141a18680`, a substantive override:
it has gates including the pawn byte `+0xb60`, then calls
`0x14232ff70(Pawn, Weapon)` and retains its result; on role byte 2 it clears a
32-bit field at `+0x2688` and performs additional controller, current-weapon,
allocation and callback work. Raw listing of its first 1500 bytes is
`/tmp/gameplay-weapon-equip/equip-virtual.asm`; its first return is at
`0x141a189be`. These observations are not a complete reconstruction. In
particular `0x14232ff70` and the role-2 path must be recovered to determine all
mutations, callbacks, and lifecycle behavior. Other pawn overrides must remain
virtual. The success result alone does not prove a particular assignment.

Smallest remaining blockers to a faithful implementation are: recover the two
now-named event bodies; recover the controller resolver and the
three interface/weapon virtual contracts; recover the base equip operation
`0x14232ff70` and the Athena override's full conditional side effects; and map
the virtuals for applicable subclasses. No source, generated native, or existing
evidence file was changed by this investigation; no build or runtime test was
run.
