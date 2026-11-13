# Native per-instance constructor sentinels

These assignments recover unconditional stores from the original 12.60 native
constructors. They execute when the native class or an instance is constructed,
before Unreal applies serialized Blueprint defaults and instance overrides. The
startup CDO repair alone is insufficient: it changes loaded CDOs after native
construction and does not ensure all native properties are copied to instances.

| Class | Original constructor | Field(s), value | Store |
| --- | --- | --- | --- |
| AFortGameStateZone | 0x141e8aa60 | WaitingToLeaveZoneTimeLeft = -1; bDBNODeathEnabled = true | 0x141e8b15d; 0x141e8b19f |
| AFortPawn | 0x1422fce30 | DamageZoneActiveBitMask = 255 | 0x1422fd743 |
| AFortPlayerPawn | 0x1422fe830 | BuildingState = None (2) | 0x1422ffb6b |
| AFortPlayerController | 0x142364800 | OverriddenBackpackSize = -1 | 0x1423661a8 |
| AFortPlayerPawnAthena | 0x141a08ef0 | ServerWorldTimeRevivalTime = -1.0 | 0x141a08f7c |
| AFortPlayerState | 0x1423f9fd0 | InitialExperienceLevel and InitialExperienceAmount = -1 | 0x1423fa6e2 |
| AFortWeapon | 0x14268d9b0 | WeaponLevel = -1 | 0x14268de54 |

The player-state store is an eight-byte all-ones write at original offset 0x384;
the two reflected int32 fields occupy 0x384 and 0x388. The zone boolean store
is a two-byte write of 0x0100; the reflected boolean occupies the second byte.
The pawn store is the join point following optional one-time name initialization.

Register `finalize` after the default-subobject and hand-written constructor
policies, so it merges into their constructors. It checks the expected parent,
model and rendered declarations, a unique compatible constructor, and rejects
competing constructor field references. Constructor assignments also work for
C++14 bitfields. Build and runtime validation belong to the combined batch.
