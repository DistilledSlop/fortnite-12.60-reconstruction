// Helper 0x141eb9fa0 on AFortAthenaGoatVehicle (static). Sole caller: GetGolfCartReverseActionName 0x1418ece10.
// Logic (asm): Input = NativeGetInputData(); null -> None. If Controller && UsesGamepad(Controller) or Flag==1 -> 0x3f0 slot, else the fallback.
// 0x3f0 is not covered by a reflected member (raw 8-byte Name read; 0x3e0 = GamepadGolfCartEBrakeRightName ends at 0x3e8)
// ABI: RCX=Context (UFortGlobalUIContext receiver), RDX=hidden return buffer (FName, zeroed first), R8=Controller, R9B=Flag.
// UsesGamepad(0x142390950) is hinted as a UFortGlobalUIContext member, but the asm passes Controller as its first
// argument with no receiver; it is called on Context with Controller as its parameter (its body uses only Controller).
// Evidence: porting/generated/native273-callees/0x141eb9fa0.asm, callers in porting/generated/native107-impl/.
// CONFIDENCE: MEDIUM
UFortGlobalUIContext* UIContext = reinterpret_cast<UFortGlobalUIContext*>(Context);
UFortInputData* InputData = UIContext->NativeGetInputData141fc0b30();
if (InputData == nullptr)
{
    return FName();
}
bool bGamepadBinding = false;
if (Controller != nullptr)
{
    bGamepadBinding = UIContext->NativeUsesGamepad142390950(Controller);
}
if (bGamepadBinding || Flag == 1)
{
    return *reinterpret_cast<const FName*>(reinterpret_cast<const uint8*>(InputData) + 0x3f0);
}
return FName();
