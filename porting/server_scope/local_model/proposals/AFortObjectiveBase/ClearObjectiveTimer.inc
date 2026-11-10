// CONFIDENCE: high
// Native exec 0x142a03f00 (AFortObjectiveBase::ClearObjectiveTimer, flags 0x4080405, no params).
// Captured body = null-check of TimerComponent (reflected Object member at 0x2b0) followed by an
// inlined copy of UFortMissionTimerComponent::ClearMissionTimer (0x1429ef920). The two decompiles
// are instruction-identical modulo that wrapper, proving the original source delegated to the
// component method rather than duplicating the timer-clear logic.
if (TimerComponent)
{
	TimerComponent->ClearMissionTimer();
}
