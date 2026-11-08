// Helper 0x141eb9e80 on AFortAthenaSKPushVehicle (static). Sole caller: GetShoppingCartCoastActionName body 0x1418ee470.
// Logic (asm): Input = NativeGetInputData() (receiver = Context); null -> None; else 8-byte FName at Input+0x3a8.
// 0x3a8 is not covered by a reflected member (0x3a0 QuickHealingName ends at 0x3a8); raw Name read.
// ABI: RCX=Context, RDX=hidden return buffer (FName), return value written and returned in RAX.
// CONFIDENCE: MEDIUM
UFortGlobalUIContext* UIContext = reinterpret_cast<UFortGlobalUIContext*>(Context);
UFortInputData* InputData = UIContext->NativeGetInputData141fc0b30();
if (InputData == nullptr)
{
    return FName();
}
return *reinterpret_cast<const FName*>(reinterpret_cast<const uint8*>(InputData) + 0x3a8);
