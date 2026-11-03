# Strict confirmation imports: dependency status

Not built, not run, not committed. Evidence roots: `porting/generated/`.

## Changed in this pass
- `ConfirmationImportProviders.h`: the resolver is now `const FNativeConfirmationManagerPair* NativeConfirmationResolveManager14514eda0(UObject*)`.
  Evidence: `frontend-native-evidence/native-ui-startup-18/root-native-scan.asm` (14514eda0 at line 16972; caller at line 8368).
- `ConfirmationImportProviders.cpp`: the invented `UFortUIDirectorSubsystemBase::NativeGetConfirmationManagerPair` call was removed.
  The resolver is now declared only (blocked).
- `action-definitions.inc`: `Activate` uses the pointer pair.

## Still blocked
1. Resolver body (14514eda0): needs the vtable slot 0x150 target (called on the argument), the +0x188 member's type,
   and the tail-call target 141e63a20. None are in generated evidence. The failure path returns a static pair at 0x148168bd8.
2. IFortUIManagerInterface installation on original owners: the native interface has no UFUNCTIONs. The slot +0x40
   signature is still a candidate. `athena-manager-definitions.inc` depends on unverified `PlayerWidgets` and
   `NativeAppendConfirmation145171a60` names.
3. Not blocked, but unverified: the `UIManagerInterfaces` TMap in `UFortUIManager_Campaign` is a possible owner storage.
   Check it against the director/manager chain before wiring.
4. Remaining dependencies (client-bot flags, Slate load guard construction, navigation/scroll helpers) are listed in
   `porting/frontend/confirmation_window_lifecycle_runtime/NOTES.md`.
