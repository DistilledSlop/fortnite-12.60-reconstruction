# Original pawn capsule initialization

The selected projection restores AFortPawn constructor 0x1422fce30 stores at 0x1422fd7d0 (42.0 radius) and 0x1422fd7da (88.0 half-height), followed by its FortPawnCapsule profile call at 0x1422fd83e. Offsets +0x404/+0x400 are independently confirmed by Athena native setters 0x141a23e40/0x141a23e00. Use the native constructor so authored derived defaults, including the observed 75 half-height, apply afterward. This projects only these constructor writes; Athena resize producers and replication callbacks are separate work.

The policy requires the original ACharacter parent and a unique existing constructor, and rejects competing capsule initialization. Runtime checkpoint 97 independently measured original-client radius42/half-height75 and server radius34/half-height75/profilePawn before this repair.
