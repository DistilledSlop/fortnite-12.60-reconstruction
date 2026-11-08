// Original body 0x14513f8d0 (thunk 0x1452eda30). Offset evidence: this+0xb70 is passed as the Context of NativeHelper1450e86c0 (0x1450e86c0), which writes OutKey and OutIndex. OutIndex is broadcast on OnRequestViewInAllEvent (reflected at +0xba8, FOnRequestSquadSlotAction, one int32 parameter).
// CONFIDENCE: HIGH
uint64 OutKey = 0;
int32 OutIndex = 0;
UFortSquadSlotSelectorButton::NativeHelper1450e86c0(reinterpret_cast<uint8*>(this) + 0xb70, &OutKey, &OutIndex);
OnRequestViewInAllEvent.Broadcast(OutIndex);
