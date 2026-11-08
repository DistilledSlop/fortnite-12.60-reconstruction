// Helper 0x141d92340 on UCaptureComponent. Callers: OnRep_CaptureProgress (0x141d8bb10), OnRep_SmoothProgressState (0x141d8bf50); both pass RCX=this.
// Evidence: [this+0xa0] (qword) != 0 gates the body; UFortMinigameObjectiveComponent::NativeHelper1440b2120(this) must return < 3.
// UCOMISS of [this+0x248] (float) against SmoothedProgress [this+0x214]: equal or unordered (ZF set) skips the body.
// Otherwise SmoothedProgress [0x214] and OldSmoothedProgress [0x210] receive the sampled value, int32 [this+0x218] = 0,
// then OnCaptureProgressUpdate ([this+0x200], float param) is broadcast with the sampled value.
// CONFIDENCE: MEDIUM
const uint8* GateObject = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(Capture) + 0xa0);
if (GateObject == nullptr)
{
	return;
}
if (UFortMinigameObjectiveComponent::NativeHelper1440b2120(Capture) >= 3)
{
	return;
}
const float SampledProgress = *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(Capture) + 0x248);
const float PreviousSmoothed = Capture->SmoothedProgress;
if (SampledProgress == PreviousSmoothed || SampledProgress != SampledProgress || PreviousSmoothed != PreviousSmoothed)
{
	return;
}
Capture->SmoothedProgress = SampledProgress;
Capture->OldSmoothedProgress = SampledProgress;
*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(Capture) + 0x218) = 0;
Capture->OnCaptureProgressUpdate.Broadcast(SampledProgress);
