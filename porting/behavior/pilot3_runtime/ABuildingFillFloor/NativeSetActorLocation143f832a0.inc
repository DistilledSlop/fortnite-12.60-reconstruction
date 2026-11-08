// Helper 0x143f832a0: AActor::SetActorLocation(NewLocation, bSweep, OutSweepHitResult, Teleport).
// Evidence: lookup_offset AActor 0x130 = RootComponent. With a root: delta = NewLocation(RDX) - [Root+0x1d0] (component location),
//   quat copied from [Root+0x1c0], vtable slot 0x470 = USceneComponent::MoveComponent(Delta, Quat, bSweep=R8, Hit=R9, flags 0, Teleport=stack byte).
//   Without a root: *Hit (R9) = FHitResult() (Time = 1.0f from .rdata 0x1459a86f8 = 0x3f800000, rest zero) and return false.
// CONFIDENCE: MEDIUM
return Actor->SetActorLocation(NewLocation, bSweep, OutSweepHitResult, static_cast<ETeleportType>(TeleportMode));
