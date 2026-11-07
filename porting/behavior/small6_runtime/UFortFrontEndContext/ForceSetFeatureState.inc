// Behaviour snippet: UFortFrontEndContext::ForceSetFeatureState (body only; spliced into the generated member function).
// Original address: 0x144cc5f20
// Evidence: CMP DL,0xb; JNC exit rejects Feature>=11 (Features[11]). Element stride 0x18 (LEA RDX,[RAX+RAX*2]; *8). Byte +0x1c9 = element+1 receives R8B (State), +0x1ca = element+2 receives R9B (Reason); Features at 0x1c8 (FFortFrontEndFeatureStruct).
// CONFIDENCE: MEDIUM
if (static_cast<uint8>(Feature) >= 11)
{
	return;
}
Features[static_cast<int32>(Feature)].ForcedState = State;
Features[static_cast<int32>(Feature)].ForcedStateReason = Reason;
