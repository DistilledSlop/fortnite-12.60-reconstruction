// Behaviour snippet: UFortCreativeKeyLockComponent::OnRep_RemainingKeysToUnlock (body only; spliced into the generated member function).
// Original body: 0x141d8bf20 (thunk 0x1428c3900); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOV EAX,dword [RCX+0x140] (RemainingKeysToUnlockAmount, reflected Int); ADD RCX,0x128 -> OnRemainingKeysUpdated (+0x128), FRemainingKeysChanged(int32).
// CONFIDENCE: HIGH
OnRemainingKeysUpdated.Broadcast(RemainingKeysToUnlockAmount);
