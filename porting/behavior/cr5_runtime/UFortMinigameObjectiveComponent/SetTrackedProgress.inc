// Behaviour snippet: UFortMinigameObjectiveComponent::SetTrackedProgress (body only; spliced into the generated member function).
// Original address: 0x1420a5cf0 (thunk 0x1429d36d0)
// Offset evidence: this+0x19c = ProgressPercentage (Float, reflected). this+0x168 = OnObjectiveUpdated (InlineMulticastDelegate); CALL 0x1406a9dc0 with packed {this} = Broadcast(this).
// Helper 0x1440b2120 = NativeHelper1440b2120 (declared static, uint8 return); the asm gates the body on its result == 3.
// CONFIDENCE: HIGH
if (UFortMinigameObjectiveComponent::NativeHelper1440b2120(this) == 3)
{
	ProgressPercentage = Percentage;
	OnObjectiveUpdated.Broadcast(this);
}
