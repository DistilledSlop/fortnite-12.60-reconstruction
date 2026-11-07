// CONFIDENCE: high
// Original address: 0x14194a330
// 0x14194a330, IFortVehicleInterface byte slot 0x3b0.
APlayerState* State = Pawn ? Pawn->GetPlayerState() : nullptr;
if (!State)
{
    State = GetPlayerState();
}
return Cast<AFortPlayerStateZone>(State);
