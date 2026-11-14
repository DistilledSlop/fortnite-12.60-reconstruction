// CONFIDENCE: HIGH. Complete original body; native descendant dispatch inventoried.
// 0x141f821f0 via interface thunk 0x14084b700. Original assumes the seat component exists.
AActor* SeatOwner = NativeGetSeatComponent163()->GetOwner();
return SeatOwner && SeatOwner->GetLocalRole() != ROLE_Authority && SeatOwner->GetOwner();
