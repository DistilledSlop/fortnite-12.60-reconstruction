// Body 0x142382cc0 (thunk 0x142a47770). ClientQuickBars +0x14d0 (reflected AFortQuickBars*); bool gate at +0x14d8 (recovered gap member NativeRecoveredBool_0x14d8).
// Storage helper 0x14205b290 = AFortPlayerController::NativeGetQuickBarSlotStorage14205b290; int32 read at returned storage +0x18 (pointee not reflected, raw).
// CONFIDENCE: HIGH
static_assert(sizeof(AFortQuickBars) > 0, "AFortQuickBars include for quickbar storage helper");
if (ClientQuickBars && NativeRecoveredBool_0x14d8)
{
    void* Storage = AFortPlayerController::NativeGetQuickBarSlotStorage14205b290(ClientQuickBars, static_cast<uint8>(QuickBarType));
    return *reinterpret_cast<const int32*>(static_cast<const uint8*>(Storage) + 0x18);
}
return 0;
