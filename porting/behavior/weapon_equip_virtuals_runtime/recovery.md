# Weapon equip virtual family: native targets and predicate

Evidence source: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, mapped at `0x140000000`. AFortWeapon class registration `0x142b43730` supplies constructor wrapper `0x142b42830`, which tail-calls `0x14268d9b0`. That constructor installs the primary vtable `0x14620a008`. Its +0x778, +0x780, +0x798 slots are respectively `0x1426aafb0`, `0x1426a8510`, `0x1426b4ce0`. These are original Windows offsets, not reconstructed layout offsets.

## +0x798: predicate, full observed control flow

The old-weapon argument is not read by this base function. The apparent source-level contract, using semantic placeholders only where necessary, is:

```cpp
bool AFortWeapon::Predicate798(AFortWeapon* /* OldWeapon */)
{
    UWorld* World = GetWorld(); // virtual call, made even if later paths do not need it
    const bool bReplay = World && World->IsPlayingReplay();
    AFortPawn* Pawn = Cast<AFortPawn>(GetOwner());
    UAbilitySystemComponent* ASC = Pawn ? Pawn->GetAbilitySystemComponent() : nullptr;
    if (NativeIsBROnly141ed8a30() && WeaponData &&
        HasAnyAbilityWeaponOrVehicleWeaponTags(WeaponData))
        return false;
    if (bReplay && Pawn && ASC && !(PawnOriginalByte521 & 0x08) &&
        ASC->GetCurrentMontage())
        return false;
    return EquipAnimation != nullptr;
}
```

`NativeIsBROnly141ed8a30` is the cached singleton selector already investigated in `porting/frontend/ui_config_singleton_runtime/RECOVERY.md`. `0x141ef9880` constructs a temporary gameplay-tag container with singleton tags +0xb90 `AbilityWeapon` and +0xb98 `VehicleWeapon`, dispatches the item definition's gameplay-tag interface subobject +0x30, virtual +0x28 with this container, and destroys the temporary. This is a virtual tag-interface query; don't replace it with raw tag pointer comparison. `0x144693a30` matches `UWorld::IsPlayingReplay` (see `aircraft_lookup_evidence.md`). `0x14070c910` exactly matches UE 4.25 `UAbilitySystemComponent::GetCurrentMontage()` including anim-instance lookup and `Montage_IsActive`. `WeaponData` +0x350 and `EquipAnimation` +0xa48 are closed by reflection. The pawn bit +0x521 mask 0x08 was not found in the pawn's reflected boolean setters, so this is not yet promotable source. The ASC is obtained through the pawn's interface subobject +0x4c8 virtual +0x10 and must retain its dispatch, including an absent component. Do not use byte offsets in reconstructed native classes or collapse the preceding calls into a simplified expression that changes call order.

## Other virtual targets and immediate effects

Both base functions are substantial: +0x778 runs at least through return `0x1426ab8f2`; +0x780 through return `0x1426a9974`. They are not callbacks alone. At entry +0x778 writes byte +0x9c4=0, broadcasts the reflected `OnSecondaryAbilityToggled` multicast at +0x2f0 with `false`, calls weapon virtual +0x758, chooses between `0x142694dd0` and `0x1426aeed0` on the observed conditional, restores material slots on `WeaponMesh` (+0x798) from `DefaultWeaponMaterials` (+0x7b0) while rechecking mesh material count each iteration, and empties that array. Further operations through the return include owner, gameplay tags/abilities, world timers and multiple virtual calls. The first two fields' source-level ownership and the remaining effects require closure before emitting the method.

+0x780 first processes member +0xb28 with `0x142ee3b30`, zeros the 8 bytes at +0x8b0 and byte +0x289, copies dword +0x8d4 (`AmmoCount`) to +0x8d8, clears +0x8dc, calls `0x142690db0`, and invokes virtual +0x5e8 (`ForceNetUpdate`). If the old pointer is nonnull it copies its bit 0 at +0xb48 into the same bit on the new weapon, then begins world/timer transfer with both worlds via virtual +0x150. There are further component, gameplay ability, callback, and timer effects through its return. These observations are ordered partial evidence, not a valid substitute body.

A pointer scan of the captured image found the following tables that share base +0x798 (this is not an exhaustive subclass census; variants overriding the predicate will not be found by this scan). Each row groups primary vtables by their +0x778/+0x780 targets. `0x1459aae40` is an unrelated false-positive table and is excluded after inspection. Table identity should be established through constructors before assigning class names.

| +0x778 | +0x780 | Captured vtables |
|---|---|---|
| 1426aafb0 | 1426a8510 | 145e4a0f0, 145e4add0, 14620a008 |
| 14267d230 | 1426a7af0 | 146203130 |
| 14267d390 | 14267b470 | 146204508 |
| 14267d480 | 14267bc50 | 146206ff0 |
| 1426aad70 | 1426a7af0 | 14620aaa8 |
| 1426aad70 | 1426a7ba0 | 14620d1a8, 14620dda8 |
| 1426d67e0 | 1426d3660 | 146214010, 146215c68, 1462175d0, 146218230, 146218ec0, 14621aee0, 14621ba50, 14621c5b0 |
| 1426d67e0 | 1426d3d10 | 1462168f0 |

For example, `0x1426a7af0` calls the base equip first, then makes additional virtual and component calls, setting +0xbda to 1; `0x1426aad70` sets that byte to 0 before base unequip and adds further component, authority and controller effects. `0x14267d480` calls `0x1428cbe60`, then tail-calls `0x14267d230`. `0x1426d67e0` calls base unequip then carries out additional world/timer and owner work. Preserve all override dispatch; do not substitute base calls for derived bodies.

## Evidence and next step

Scratch windows/listings: `/tmp/gameplay-weapon-virtuals/{1426aafb0,1426a8510,1426b4ce0}.{bin,asm}` (each binary is 0x1800 bytes); SHA-256 respectively `3f7da73a2e3118536114a0e2e772b10eef72f932eeaceeb075874e7bb5fc09b5`, `c2e9875f0bbf7137ffcd3bfa0e7bc9bfe139542925e66a33b6bc0ef89def873d`, `7abcfb1e2f8c43ae81cb8dfd10e6167edcb356924894bc43633052a32695f791`. Extract by seeking VA minus base and use `objdump -D -b binary -m i386:x86-64 -M intel --adjust-vma=<VA>`. Class/constructor windows and the bounded pointer scan program are in the same scratch directory.

Next recover the pawn +0x521/0x08 native flag and close the source-level tag helper/interface declaration, then the predicate can be emitted while preserving virtual dispatch. Recover complete base equip/unequip bodies and each applicable concrete subclass, resolving timer/delegate and ability effects before promotion. No manifest, generated native code, Ghidra database, build, or test was changed or run.
