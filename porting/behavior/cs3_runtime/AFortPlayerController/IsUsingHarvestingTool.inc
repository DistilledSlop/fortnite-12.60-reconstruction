// Body 0x142390980 (thunk 0x142a484f0). ClientQuickBars +0x14d0 (reflected) and bool gate +0x14d8 (recovered gap member NativeRecoveredBool_0x14d8). Index helper 0x142059ed0 = AFortPlayerController::NativeGetSlotIndexByKind142059ed0 with Kind 0; SlotIndex stays -1 when gated off.
// CONFIDENCE: HIGH
static_assert(sizeof(AFortQuickBars) > 0, "AFortQuickBars include for slot index helper");
int32 SlotIndex = -1;
if (ClientQuickBars && NativeRecoveredBool_0x14d8)
{
    SlotIndex = AFortPlayerController::NativeGetSlotIndexByKind142059ed0(reinterpret_cast<AFortPlayerController*>(ClientQuickBars), 0);
}
return SlotIndex == 0;
