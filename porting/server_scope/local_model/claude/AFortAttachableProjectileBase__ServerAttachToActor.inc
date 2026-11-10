// CONFIDENCE: MEDIUM. Original addresses: exec thunk 0x142842120, core helper 0x1426dedd0 (dep).
// The thunk reads the five parameters in declaration order (Hit, AttachToActor, StickyOffsetFromPhysicsMesh,
// StickyOffsetFromBoneCenter, NarrowPlacementAgainstVelocityThreshold) and passes them to the attach core.
AttachToActorImpl(Hit, AttachToActor, StickyOffsetFromPhysicsMesh, StickyOffsetFromBoneCenter, NarrowPlacementAgainstVelocityThreshold);
