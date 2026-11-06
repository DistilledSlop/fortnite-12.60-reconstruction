// CONFIDENCE: high
// 0x140850640. The incoming seat argument remains in EDX for slot 0x590.
AFortPawn* SeatPawn = NativeGetPawnForFilterSeat163(Seat);
if (SeatPawn)
{
    AFortPawn* DriverPawn = NativeGetDriverForTargetFilter163();
    if (DriverPawn)
    {
        const uint8 DriverTeam = DriverPawn->NativeGetTeam163();
        const uint8 SeatTeam = SeatPawn->NativeGetTeam163();
        return DriverTeam == SeatTeam;
    }
}
return true;
