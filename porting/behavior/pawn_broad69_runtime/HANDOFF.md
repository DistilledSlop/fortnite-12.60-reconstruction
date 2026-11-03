# Pawn broad 69 draft

Eight inline behavior transcriptions, uncounted and not source-complete. No manifest promotion, compilation, build, tests, or Ghidra. Adapter types and calls are unresolved; offset-based code is intended for review, not compilation/splicing. Source: matching `porting/generated/decomp_improved/<class>/<function>.c` and `porting/generated/decomp/<class>/<function>.c` (generated inputs are git-ignored). P_FINISH is exec plumbing and omitted from native sketches.

| Class / function | Exec address | Principal unresolved effects |
| --- | --- | --- |
| AFortPawn::OnRep_PushMomentum | 142a1ec60 | Movement vtable calls, exact rsqrt and global vector; preserve rereads and component flag changes. |
| AFortPawn::OnRep_bWeaponHolstered | 142a1ece0 | Weapon and pawn calls and the two delegate offsets; weapon may change during callbacks. |
| AFortPawn::KillDisconnectedPawn | 142a1ce20 | Tag storage, ownership of copied container through vcall a78, globals and helper signatures; cleanup only shown on original storage. |
| AFortPlayerPawn::OnRep_FacialTypeOverride | 142a7dd30 | Component lookup arguments and class tests; exact 12-byte field copies. |
| AFortPlayerPawn::OnRep_IsInsideSafeZone | 142a7de10 | Interface receiver, map key/value and tag count mutation; broadcast parameter layout. |
| AFortPlayerPawn::OnRep_IsInAnyStorm | 142a7ddd0 | World and predicate calls, effect cleanup, terminal vcall. |
| AFortPlayerPawn::OnRep_IsParachuteForcedOpen | 142a7de30 | Sound spawning signature, global rotation, audio vcall's upper pointer bits, controller predicate. |
| AFortPlayerPawn::OnRep_PossessedProp | 142a7e0c0 | Delegate payload layout, component lookup and class test. |

Bounded duplication search `rg -l 'OnRep_PushMomentum|LaunchCharacterJump|GetTargetingTransform|SetGravityJumpMultipliers|OnRep_bWeaponHolstered|KillDisconnectedPawn|OnRep_FacialTypeOverride|OnRep_GliderOverride|OnRep_IsInsideSafeZone|OnRep_IsInAnyStorm|OnRep_IsParachuteForcedOpen|OnRep_PossessedProp' porting/behavior` returned no pre-existing files before drafting. Used bounded size listing, improved and raw decomp excerpts; no runtime claims. `LaunchCharacterJump` and `SetGravityJumpMultipliers` were parked as wrappers whose actual callees were not available, per assignment. Exact signatures, field bindings, float/SIMD behavior, side effects and ownership remain to be resolved before promotion. Parent retains native promotion and integration.
