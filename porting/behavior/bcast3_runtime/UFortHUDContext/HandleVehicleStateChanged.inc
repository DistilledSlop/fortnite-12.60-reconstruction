// Original body 0x144cd5130 (thunk 0x145257810). Offset evidence: ADD RCX,0x4f0 -> OnPlayerVehicleStateChanged (reflected exact at +0x4f0). Pack: [0]=RDX PlayerPawn, [8]=R8 NewVehicle, [0x10]=R9 OldVehicle.
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>: mapped to <Member>.Broadcast(...) per the broadcast addendum.
// CONFIDENCE: HIGH
OnPlayerVehicleStateChanged.Broadcast(PlayerPawn, NewVehicle, OldVehicle);
