# CommonInput module runtime candidate

This candidate replaces the generated default module. The registration from
`CommonInput` to factory `14081bef0`, its vtable, the startup outer and log
category are established in [module identity evidence](../common_input_module_identity_evidence/HANDOFF.md).
Startup creates a distinct `UCommonInputSettings` with the transient package as
outer, default name and flags, stores it, and roots it. It is not the class
default object.

Shutdown `1407fb490` reads byte `148247293`: if zero it dereferences the stored
settings and removes its root, then clears the field; if nonzero it clears the
field without dereferencing it. The global is `GExitPurge`: the same byte is
set to 1 at `142e8de6a`, in `StaticExit` immediately after the unreachable-object
mark loop and before the final garbage purge, matching `Obj.cpp`'s
`GExitPurge = true`. No settings-null check belongs in shutdown.

Accessor `1407c4fc0` uses `IsInGameThread()`: on a non-game thread it calls
`142cbd3c0`, matching `FModuleManager::GetModule`; on the game thread it calls
`142cc0750` (a jump to `142cc0600`), matching `FModuleManager::LoadModule`.
The latter has the engine's own non-game-thread GetModule branch and otherwise
calls LoadModuleWithFailureReason. Both branches then dispatch module vtable
+0x40 without a null check. Direct pointer APIs preserve the observed shipping
behavior, including a failed lookup; no extra checked-API assertion is inserted.
The module class and helper names are source aliases, not recovered symbols.
`LogCommonInput` is defined once in the module and declared in the header.

Generator integration: load `policy.py`, call `validate_source(image_path)` on
the mapped original image, then call `sources('CommonInput', blocks_by_va,
name_to_block)` only for a promotion that includes the original validated settings
class; it fails closed on its complete frozen class/property record. Write the
returned `CommonInput.cpp` as `CommonInput/Private/CommonInput.cpp` instead of
`render_module_cpp`'s default, and `CommonInputModuleRuntime.h` into
`CommonInput/Generated/` so consumers share the declaration. Never silently
fall back to the default module if validation fails. Existing cache code may
include this header or use its compatible extern log declaration. This candidate
has not modified generator, generated native files, or build configuration.
The bounded fingerprints cover the registration, log category, getter, factory,
startup, shutdown, vtable, transient-package lifecycle and relevant literals;
they do not replace compilation or runtime verification.
