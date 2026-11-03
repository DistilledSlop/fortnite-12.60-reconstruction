# Projectile native98 checkpoint

Eight bodies are ready for integration: two AFortProjectileBase replication callbacks, AddTagToDefaultTags, and five movement setters. Generated class includes should be inferred; movement setter bodies using AActor require the Engine `GameFramework/Actor.h` include. FlushNetDormancy identification is documented in `porting/behavior/building_asc54_runtime/HANDOFF.md`; tag helper identification is documented in `porting/behavior/athena_quick_native48_runtime/HANDOFF.md`.

Remaining blockers:

- EnterSkyTube, ExitSkyTube, GetDefaultGravityScale, GetDefaultSpeed, GetOwnerPlayerController, GetOwnerWeapon and SetGravityScale require unreflected fields, additional helper identification, or virtual slot identification.
- OnBounceCallback and OnStopCallback dispatch through virtual slots; the two Athena RPC wrappers also dispatch validation and implementation through virtual slots.
- OnRep_CurrentSkyTube requires identifying its object state and helper behavior; OnRep_ResumeSimulationCount and OnRep_SimulationStoppingHit require virtual slot identification.
- Movement OnRep_HomingData, OnRep_FortStopSimulatingRepData, OnRep_ReplicatedAutoRegisterUpdatedComponent, OnRep_ReplicatedVelocity, SetHomingData, SetHomingTarget, SetHomingTargetPosition, SetReplicatedAutoRegisterUpdatedComponent, and SetVelocity require identification of virtual dispatch and/or helper behavior.

Wrapper bodies are under `porting/generated/native98-broad`; helper bodies are under `porting/generated/native98-impl`. No tests run, per task instruction.
