// CONFIDENCE: MEDIUM. Original address: 0x142834070 (exec thunk). Helper chain: 0x141919b80 -> 0x1440b5c60 (RaycastSingle).
// Behaviour: a single world raycast from TraceStart to TraceEnd with the "GroundTrace" tag and default params. The out hit is
// initialised with Time = 1.0 (no hit) before the trace, and the raycast result is the return value.
OutHit = FHitResult(1.0f);
FCollisionQueryParams Params(FName(TEXT("GroundTrace")), /*bTraceComplex*/ false);
return GetWorld()->RaycastSingle(OutHit, TraceStart, TraceEnd, Params);
