// Behaviour snippet: AFortMissionState::UpdateUI (body only; spliced into the generated member function).
// Original body: 0x1420f5fc0 (thunk 0x142737e10); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: CMP dword [RCX+0x228],0; JLE 0x1420f5fe3 (epilogue inside body). 0x228 is Num of the invocation list at delegate +0x8 (OnUpdateUI at 0x220).
//   The guard is written as IsBound(); the body stores this (RCX) in the params slot; FOnUpdateUI(AFortMissionState* MissionState).
// CONFIDENCE: MEDIUM
if (OnUpdateUI.IsBound())
{
    OnUpdateUI.Broadcast(this);
}
