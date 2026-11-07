// Original body address: 0x141f3ac80 (thunk 0x1429f03c0)
// Offset evidence: no member access; receiver RCX stays this (this_delta 0).
// Callee 0x141f306a0 -> UFortMissionTimerComponent::GetMissionTimerTimeRemaining() const (declared in UFortMissionTimerComponent.h).
// Body: CALL callee; COMISS XMM0,0.0; SETNC AL, i.e. result is (remaining >= 0.0f), NaN gives false.
// CONFIDENCE: HIGH
return GetMissionTimerTimeRemaining() >= 0.0f;
