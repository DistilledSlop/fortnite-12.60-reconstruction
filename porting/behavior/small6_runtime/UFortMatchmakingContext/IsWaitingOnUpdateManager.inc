// Behaviour snippet: UFortMatchmakingContext::IsWaitingOnUpdateManager (body only; spliced into the generated member function).
// Original address: 0x1421a31d0
// Evidence: CMP qword ptr [RCX+0x1a8],0; SETNZ AL. Reflected OnOutpostDiscoveryFailure is at 0x198 (delegate, 0x10 bytes assumed); 0x1a8 not reflected -> NativeRecoveredPointer_0x1a8.
// CONFIDENCE: MEDIUM
return NativeRecoveredPointer_0x1a8 != 0;
