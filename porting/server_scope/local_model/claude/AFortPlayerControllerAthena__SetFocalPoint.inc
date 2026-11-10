// CONFIDENCE: MEDIUM. Original exec 0x142a5d040 (UFunction thunk, no RPC validation).
// Parameters in thunk read order: InFocalPoint (object), InFocalPointOffset (FVector: x and y as 8 bytes, z as 4 bytes at +8), InFocalPointFOV (float).
// Body is FUN_1419f3e70, inlined below. Member names: FocalPoint at +0x2b40 (resolved); FocalPointOffset at +0x2b48, FocalPointFOV at +0x2b54 (proposed, unresolved).
if (InFocalPoint != nullptr && FocalPoint != nullptr && LogFortPlayerVerbosity > 2)
{
	UE_LOG(LogFortPlayer, Warning, TEXT("SetFocalPoint %s stomping previous focal point %s"), *InFocalPoint->GetNameForLog_0x142ce9f90(), *FocalPoint->GetNameForLog_0x142ce9f90());
}
FocalPointOffset = InFocalPointOffset;
FocalPointFOV = InFocalPointFOV;
FocalPoint = InFocalPoint;
