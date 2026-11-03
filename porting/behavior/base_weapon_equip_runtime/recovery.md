# Base weapon equip transition, 0x14232ff70

Evidence: image-125 runtime mapped at 0x140000000. `/tmp/gameplay-base-equip/base.asm` is GNU objdump of the 4096-byte entry window, SHA-256 `673b45293e9f546e3559333f39be086656d5f16e50060650f6ac9c8ce75a25e8`. Main return path ends at 0x1423304e3; bytes after that include cold exception-unwind material. All offsets below are original Windows runtime offsets, never use them as offsets in the reconstructed Linux class. This is a precise integration contract, not a compilable or complete native implementation.

The following pseudocode names unnamed virtuals by observed role only; names are not asserted original spellings. Calls retain their virtual dispatch and source order. Loads written as `CurrentWeapon` must be re-read after intervening calls: the callbacks can mutate the pawn. `incoming` and `old` are saved original local pointers. `Role` is re-read at the points shown.

```cpp
bool BaseEquip(AFortPawn* self, AFortWeapon* incoming) {
  if (incoming && incoming->Instigator && incoming->Instigator != self) {
    // Category threshold >= 3: log level 3
    // "EquipWeapon - Weapon %s is owned by %s but %s attempted to being equip it."
    // Weapon/instigator/self object names are converted during logging.
    return false;
  }
  if (self->Role == 2) {
    self->CurrentWeaponList.Remove(nullptr);  // removes ALL nulls, preserving order
    self->CurrentWeaponList.AddUnique(incoming); // including nullptr
  }
  if (incoming == self->CurrentWeapon) return false;

  if (self->v_cd8_HasChangeEquipmentTag(Tags().ChangeEquipment))
    self->v_cf0_RemoveChangeEquipmentTag(Tags().ChangeEquipment);
  if (incoming && incoming->v_798_PreserveEquipmentTag(self->CurrentWeapon)) {
    self->v_ce8_AddChangeEquipmentTag(Tags().ChangeEquipment);
  } else {
    auto& tags = Tags();
    self->v_ce8_AddChangeEquipmentTag(tags.ChangeEquipment);
    self->v_cf0_RemoveChangeEquipmentTag(tags.ChangeEquipment);
  }
  AFortWeapon* old = self->CurrentWeapon;
  if (old) {
    old->v_778_Unequip();
    self->CurrentWeapon = nullptr;
    if (!incoming) self->v_c48_UpdateWeaponState();
    self->OnWeaponUnEquippedDelegate.Broadcast();
  }
  // Fetch via interface virtual at pawn+0x4c8, slot +0x10. If it is a
  // UFortAbilitySystemComponent, invoke its virtual HandleGameplayEvent:
  // tag = Tags().WeaponSwitch; default FGameplayEventData has Instigator=self,
  // Target=self, OptionalObject=incoming, OptionalObject2=old. Its EventTag
  // member itself remains default. The event occurs before new assignment.
  if (auto* asc = Cast<UFortAbilitySystemComponent>(GetAbilitySystemViaInterface(self))) {
    FGameplayEventData event;
    event.Instigator = self; event.Target = self;
    event.OptionalObject = incoming; event.OptionalObject2 = old;
    asc->HandleGameplayEvent(Tags().WeaponSwitch, &event);
  }
  if (incoming) {
    AFortPlayerController* pc = Cast<AFortPlayerController>(self->Controller);
    if (pc && pc->v_6a8_IsLocalController() && pc->bTargetingToggleable)
      self->bSecondaryInputHeld = false;
    self->CurrentWeapon = incoming;
    self->ForceNetUpdate();
    if (self->v_b78_ShouldEquipWeapon()) {
      if (self->Role != 3 && !self->CurrentWeapon->GetOwner())
        self->CurrentWeapon->SetOwner(self);
      self->CurrentWeapon->v_780_Equip(old);
    }
    self->v_c48_UpdateWeaponState();
    if (pc && (pc raw byte +0x3c8 & 2))
      self->CurrentWeapon->SetActorHiddenInGame(pc raw byte +0x62a);
  }
  GlobalWeaponChangeNativeMulticast.Broadcast(self, self->CurrentWeapon, old);
  return true;
}
```

## Closed identities and constraints

* At 0x141ebff99–0x141ebffaa the shared lazy native tag singleton (`0x141ed1800`) registers ASCII `Gameplay.Action.Player.ChangeEquipment` into +0x6c0. At 0x141ebeb53–0x141ebeb64 it registers `Event.Weapons.WeaponSwitch` into +0x160. Both are part of the existing singleton initialization sequence; registration on first equip or eager independent registration changes timing/finalization. See `playlist_mode_evidence.md`.
* Array helpers `0x140726fc0` and `0x14085a990` respectively perform stable removal of all equal pointers (without shrink) and AddUnique. A simulated proxy equipping nullptr will first remove all nulls then add one null, even when the operation returns false.
* +0x9c0 is reflected `OnWeaponUnEquippedDelegate`, no parameters; `0x1406a9dc0` is script multicast processing. `0x140aae720` at global `0x1478de548` is a native multicast broadcast with arguments pawn/current/old, reverse iteration and mutation-safe lock/compaction; preserve a real native multicast, including reentry and listener lifetime. Its original symbol name is not established.
* The pawn +0x4c8 subobject virtual +0x10 is the ability system interface getter. The component class getter `0x14277e480` embeds `FortAbilitySystemComponent`. Gameplay event virtual +0x620 and FGameplayEventData layout match UE 4.25. Do not replace this with a generic ASC cast: the original requires the Fort subclass.
* `0x142390770` reads controller byte +0xad0. Reflection bool bits identifies this as `bTargetingToggleable`. Pawn +0x522 mask 0x20 is `bSecondaryInputHeld`. Controller +0x6a8 corresponds to local-controller predicate; original checks only when controller is AFortPlayerController. Actor +0x2f8 is SetActorHiddenInGame in the UE vtable model, but exact controller fields at +0x3c8 and +0x62a are still unmapped.
* AFortPawn base vtable 0x146115880 and captured Athena table 0x145e2cfd0 agree on +0xcd8 = 0x142331eb0, +0xce8 = 0x142353060, +0xcf0 = 0x142353960. These forward to pawn +0xbb8 if present, else return false/no-op, tail-jumping respectively to 0x141603a40, 0x14160f8c0, 0x14160fd30. Their `Has`/`Add`/`Remove` names above are semantic hypotheses; establish underlying component API before implementation. Preserve virtual overrides.
* Base +0xb78 = 0x142304290 returns Role != 1. Athena +0xb78 = 0x1423042a0 additionally accepts Role 1 if 0x142331940 returns true. Base +0xc48 = 0x14234f080 is substantive; Athena +0xc48 = 0x14234f2a0 calls base plus additional behavior. Neither may be replaced by a notification stub. +0x5e8 is ForceNetUpdate according to the validated vtable model.
* Ownership diagnostic uses incoming +0xe0 (AActor Instigator), whereas later owner check uses GetOwner (+0x118). Do not conflate these fields. The warning path includes object-name allocation/conversion and is conditional on the original logging category threshold.

## Integration boundary

The body cannot be safely promoted yet. Required concrete inputs are: implement and expose native virtuals +0xcd8/+0xce8/+0xcf0, +0xc48, +0xb78 and weapon +0x778/+0x780/+0x798 for relevant subclasses; map controller +0x3c8 bit 1 and +0x62a; provide the shared tag singleton in original registration order; recover the global native multicast owner and subscription paths. The +0xc48 base and Athena targets above are the exact next dependencies; +0xbb8 component forwarding targets are listed above. Do not substitute no-ops or call dynamic events in their place. No shared generator, manifest, native code, build or Ghidra database was modified; no runtime tests or native builds were run.
