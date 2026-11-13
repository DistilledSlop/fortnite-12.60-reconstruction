// Original body 0x141be59c0 (thunk 0x142740c50) for ABuildingFillFloor::OnRep_FloorZ.
// Offset evidence: [RCX+0xf0] is AActor::Role (private, GetLocalRole(); 3 = ROLE_Authority); [RCX+0x130] = AActor::RootComponent; [Root+0x1d0..0x1d8] = world location floats (ComponentToWorld translation; GetComponentLocation); [RCX+0x7ec] = FloorZ (reflected float).
// Null root falls back to the ZeroVector snapshot (0x148218630/38); only X,Y of the loaded vector are used, Z is replaced by FloorZ.
// CONFIDENCE: MEDIUM
if (GetLocalRole() == ROLE_Authority)
{
	return;
}
FVector NewLocation = FVector::ZeroVector;
if (RootComponent != nullptr)
{
	NewLocation = RootComponent->GetComponentLocation();
}
NewLocation.Z = FloorZ;
ABuildingFillFloor::NativeSetActorLocation143f832a0(this, NewLocation, false, nullptr, 0);
