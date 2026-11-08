// Behaviour snippet: UFortMinigameObjectiveComponent::SetTrackedState (body only; spliced into the generated member function).
// Original address: 0x1420a5d40 (thunk 0x1429d3750)
// Offset evidence: this+0x1a0 = TrackedState (Byte) written from DL (InState, ABI arg 2). this+0x1a1 = TrackedStateOwnerTeam (Byte) written from R8B (InStateOwnerTeam, ABI arg 3). this+0x178 = OnObjectiveStateChanged (InlineMulticastDelegate); CALL 0x1406a9dc0 = Broadcast(this).
// Helper 0x1440b2120 = NativeHelper1440b2120 (declared static, uint8 return); the asm gates the body on its result == 3.
// CONFIDENCE: HIGH
if (UFortMinigameObjectiveComponent::NativeHelper1440b2120(this) == 3)
{
	TrackedState = InState;
	TrackedStateOwnerTeam = InStateOwnerTeam;
	OnObjectiveStateChanged.Broadcast(this);
}
