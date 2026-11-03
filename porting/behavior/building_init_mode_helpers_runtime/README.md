# Building initialization mode helpers

Two source bodies and one actual native field are proposed, without canonical edits, build, tests, or runtime claims. `manifest-entries.json` uses address aliases, not recovered original names. Static ownership of 141ed4e30 is assigned for integration. The byte field must be joined with the actual GI lifecycle and its writers, not duplicated as a cache. Constructor initialization to 2 is supported, but full writer/transition integration remains open. Existing `porting/frontend/game_instance_subgame_runtime` is an unregistered candidate for the same byte and accessor, and must be reconciled rather than installed twice.

Evidence pins:

* `porting/generated/callee_probe/building_vitals_dependencies_2/0x141ed4e30.asm` and `.c`: non-null context calls 144640410 with GEngine and mode 0; World+188 is read, class-checked via 1428ea7f0, then 141e5f160 is tail-called. Otherwise engine and manager are checked, 141df62a0 is called and manager+6cc returned, or 2 is returned. No initial GEngine guard precedes the non-null context call.
* `porting/generated/callee_probe/building_vitals_dependencies_3/0x141e5f160.asm`: zero-extended byte at receiver+828. `porting/behavior/building_vitals_runtime/README.md` pass 5 identifies 1428ea7f0 as UFortGameInstance and World+188 as OwningGameInstance; `UWorld::GetGameInstance()` reads that field.
* `porting/frontend/game_instance_subgame_runtime/README.md`: constructor 141e48c70 sets +828 to 2 and documents known setter 141e7ec30 and additional writer 141e648aa, with transition dependencies still open.
* `porting/behavior/asset_load_callback_runtime/NativeGetGameSubMode.inc`, canonical UFortAssetManager: the fallback has the same engine/manager guards, checked manager lookup 141df62a0 and original +6cc read. Do not replace its fatal checked lookup with an unchecked cast.
* Engine `Engine.h` defines `EGetWorldErrorMode::ReturnNull` as enum value 0. The native lookup is deliberately not changed to `GetWorldFromContextObjectChecked`.

141ed2350 and the UFortEngine+199 byte are not included: this bounded closure does not require them. Existing frontend configuration candidate declares an unrelated no-argument external alias for 141ed4e30; integration should call this helper with nullptr there.
