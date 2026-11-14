# Original ServerMove receive scope candidate

Complete typed lift of original `UFortMovementComp_Character::ServerMove_Implementation` at `0x1420ed7e0`. This is the receive implementation, **not** the client `ServerMove` sender. Both original Fort Character and Athena movement primary vtables (`0x1460a98e0`, `0x145e1db40`) have this target at `+0x9f0`. Original Character exec `0x1446e7d20` calls `0x144088110`, which obtains CharacterMovement and dispatches precisely that slot. The original override directly calls stock `UCharacterMovementComponent::ServerMove_Implementation` at `0x1440c8670`.

The apparently impossible acceleration Z is an intentional wave time envelope: Z >= 90000 carries `(Z-90000)*float_bits(0x3dcccccc)`. The original clears only this encoded Z, then runs stock movement with temporary WaterSubsystem state and restores both previous values afterward. It sets the override-enabled flag before examining Z, even if Z is below the sentinel. It still clears encoded Z if the world or WaterSubsystem is absent. The CVar controls the temporary enabled flag; it does not control decoding or clearing Z.

Integration requires admitting the manifest candidate as a virtual public helper override, adding the complete body, and emitting `cpp_support.inc` exactly once in the owning cpp after the own-header include. The plugin dependency already exists in FortniteGame.Build.cs. No Engine, Native, generator, canonical MANIFEST, build, staging, Git, or live changes were made here.

The required service helpers are already complete in the project Water plugin; do not duplicate state or approximate them:

| Original | Typed existing plugin API |
| --- | --- |
| `0x1414e6ca0` | `UWaterSubsystem::GetWaterSubsystem(UWorld*)`: returns `World->GetSubsystem<UWaterSubsystem>()`, else null |
| service `+0x70` load | `GetOverrideSmoothedWorldTimeSeconds()` |
| service `+0x75` load | `GetShouldOverrideSmoothedWorldTimeSeconds()` |
| `0x1414f5e00` | `SetOverrideSmoothedWorldTimeSeconds(float)`: assigns override time |
| `0x1414f5e10` | `SetShouldOverrideSmoothedWorldTimeSeconds(bool)`: assigns override enabled flag |

Original static class helper `0x1414fa9e0` explicitly names `/Script/Water`, `WaterSubsystem`. Original `0x1414e6ca0` implements the world subsystem collection query and class check. Original setters are one-store functions. The installed plugin has the matching getters and setters in `porting/native/Plugins/Water/Source/Runtime/Public/WaterSubsystem.h` and `Private/WaterSubsystem.cpp`; the same implementation is present in UE4260.

`Fort.Water.UseClientWaveTime` is the exact original CVar name (UTF16 at `0x14609b810`), help text is at `0x14609b760`, int32 default 1 is at `0x1478c51c0`; registration xref `0x1402ee416` passes that backing variable and flags 0 (`ECVF_Default`). Emit one registration shared by any future sender recovery, not one per subclass.

The remote-view/acceleration replication producers restored in commit `8f37b4a4` are a different lifecycle: AFortPlayerPawn PreReplication/PreReplicationForReplay pack replicated proxy view/acceleration fields. They neither declare this CVar nor implement the Character ServerMove receiver. This lift preserves those producers.

Evidence is under `porting/generated/agents/movement-wire-decode67/`: exact original decompiles for override, Character exec, Water subsystem static class/query/setters, NetQuantize10/100, sender packed vector and byte captures. `REPORT.md` records the diagnosis and limits.

This closes a real missing receive-side parity contract. It does **not** prove the ground correction loop is fixed: stock MoveAutonomous already calls ConstrainInputAcceleration, which strips Z for walking/falling before acceleration clamping. Further speed/input state diagnosis remains necessary. Build/runtime validation belongs to the root's combined batch.
