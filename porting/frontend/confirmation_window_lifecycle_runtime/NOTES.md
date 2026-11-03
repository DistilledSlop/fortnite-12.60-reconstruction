# Confirmation window lifecycle: dependency status

Not built, not run, not committed. Evidence roots: `porting/generated/`.

## Closed by evidence (fragments provided)
- Load guard forced-spinner accessor (original 1407f7e20): `load-guard-definitions.inc`,
  `load-guard-members.inc`. Evidence: `decomp_batches/run-20261031T230209Z-b008/UCommonLoadGuard/SetIsLoading__140811170.c`
  and `frontend-native-evidence/common-input-manager-queue-4/commonui-scan.asm` (1407f7e20 at about line 100788).
  The tail call 1408033f0 matches `SLoadGuard::UpdateLoadingState` per `common_lazy_image_connected_runtime/RECOVERY.md`.
- Manager pair ABI: `14514eda0` returns a pointer to `{Object, Interface}`. Evidence: caller at
  `frontend-native-evidence/native-ui-startup-18/root-native-scan.asm` line 8368. Focus wrapper and `Activate` updated.

## Still blocked
1. UCommonLoadGuard Slate construction (RebuildWidget): text, style, alignment, brush and ULoadGuardSlot content.
   Not in decomp evidence; only BP_GuardAndLoadAsset, IsLoading, SetIsLoading, SetLoadingText and ULoadGuardSlot setters exist.
   Needs a new decompile target for the RebuildWidget.
2. Client-bot flags (+0x1818 / +0x1838, bool candidates in `controller-members.inc`): no writer found in generated evidence.
   Four adhoc decompiles (142332da0, 14231a050, 14231c0e0, 142319f60) read +0x1818 as pointer-sized, so the bool
   reading may be class-specific. IsClientBot 142a47ff0 and BotClickButton 145218540 bodies are absent from generated evidence.
   Flags are left uninitialized on purpose.
3. Navigation focus 144db14e0 and scroll helper 144e1c6e0: only call sites are evidenced (root-native-scan.asm line 50819
   for 144db14e0). The wrapper 144f6bdb9 and activation 144f6bb10 cited in README are not found in generated evidence.
   UFortUINavigationManager native class has no focus method body.
4. Manager resolver 14514eda0 body: evidence shows a vtable slot 0x150 call, +0x188 member, an index/registration check,
   and a tail call to 141e63a20 (not in evidence). The previous call to
   `UFortUIDirectorSubsystemBase::NativeGetConfirmationManagerPair` was removed because no such member exists in
   `porting/native/Source/FortniteGame/Generated/UFortUIDirectorSubsystemBase.h`.
5. IFortUIManagerInterface: the native header has no UFUNCTIONs. The slot +0x40 `NativeShowConfirmation` signature is a candidate.
   UFortUIManager_Campaign exposes `UIManagerInterfaces` (TMap by local player), a possible storage target. Owner installation is unverified.
