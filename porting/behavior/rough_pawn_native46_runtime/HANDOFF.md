# Pawn and controller native survey

No implementation was promoted: the inspected substantial candidates have unresolved dependencies and cannot be represented faithfully under the no-approximation requirement. No manifest entries are proposed.

- `AFortPlayerPawn::UpdatePreviousPositionAndVelocity`: reflected exec `0x142a81000`, native `0x14235bd20`, full probe `porting/generated/callee_probe/blocking/0x14235bd20.{c,asm,json}`. Stores PreviousPosition and PreviousVelocity then tail-calls pawn vtable slot +0x1008 with the remaining argument; that virtual's identity and signature are unresolved.
- `AFortPlayerPawn::RemoveAbilityAITarget`: exec `porting/generated/decomp/AFortPlayerPawn/RemoveAbilityAITarget.c`, native `0x142347a60`, probe `porting/generated/callee_probe/blocking/0x142347a60.{c,asm,json}`. Authority and target checks, array removal, then delegate removal at target +0x928; the delegate field and its binding semantics need identification.
- `AFortPlayerPawn::AITargetDied`: native `0x142301790`, same delegate dependency as RemoveAbilityAITarget, plus class getter `0x1427a1790` needs identification.
- `AFortPlayerPawn::AddIgnoredActor`: native `0x142302250`, probe in `callee_probe/blocking`; depends on timer update `0x142347f20`, movement ignore operation `0x1440f4040`, and exact native ignored-actor entry layout and timing.
- `AFortPlayerPawn::IsEmoteBlockedByMovement`: native `0x142331230`, probe in `callee_probe/blocking`; movement and vehicle virtual slots +0x548, +0x1088, and +0x2b8 remain unresolved, including the unconditional movement component reads at the end.
- `AFortPlayerPawn::GetSlopeSlidingAngles`: native `0x142324750`, probe in `callee_probe/blocking`; non-simulated-proxy path calls `0x14230e8d0`, requiring recovery of that path and the packed-angle fields.
- `AFortPlayerPawn::IsPassengerInVehicle`, `CanShootFromVehicle`, and `IsOperatingTurret`: native `0x142331bc0`, `0x14230c630`, and `0x142331940`; vehicle interface virtuals +0x568, +0x570, and +0x560 require identified exact methods.
- `AFortPlayerControllerAthena::ServerSetRespawnData`: native `0x1419f2490`; class getter and helper calls `0x141a3d510`, `0x141a5b110` need identification.
- `AFortPlayerControllerAthena::IsMatchmaking`: native `0x1419dd6b0`; class getter and helper `0x141e613c0` need identification.

The shorter `GetHomeActor`, stamina, and related candidates were not promoted because their apparent 20-line decompiles consist primarily of type-check machinery and simple getter logic, outside the requested substantial-body focus. No Ghidra session, tests, builds, runtime, canonical changes, or commits were performed.
