// CONFIDENCE: MEDIUM. Original exec 0x142a5d160 (UFunction thunk, no RPC validation).
// Parameters in thunk read order: InFocalPoint (object), InFocalPointOffset (FVector), InFocalPointFOV (float), InDuration (float).
// Body is FUN_1419f3fa0, inlined below. Member names: FocalPoint at +0x2b40 (resolved); FocalPointOffset +0x2b48, FocalPointFOV +0x2b54, FocalPointDuration +0x2b58 (proposed, unresolved).
if (InFocalPoint != nullptr && FocalPoint != nullptr && LogFortPlayerVerbosity > 2)
{
	UE_LOG(LogFortPlayer, Warning, TEXT("SetFocalPointWithDuration %s stomping previous focal point %s"), *InFocalPoint->GetNameForLog_0x142ce9f90(), *FocalPoint->GetNameForLog_0x142ce9f90());
}
FocalPointOffset = InFocalPointOffset;
FocalPointFOV = InFocalPointFOV;
FocalPointDuration = InDuration;
FocalPoint = InFocalPoint;
