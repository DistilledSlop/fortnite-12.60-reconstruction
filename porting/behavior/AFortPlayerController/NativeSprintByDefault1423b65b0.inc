// CONFIDENCE: HIGH
// Original 0x1423b65b0; keep the short-circuit ordering.
if (bSprintByDefault || NativeUsesTouchInput1423909c0())
{
    return true;
}
AFortPlayerPawn* FortPawn = Cast<AFortPlayerPawn>(GetPawn());
return FortPawn && FortPawn->NativeSwimSprintByDefault1423508d0();
