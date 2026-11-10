// CONFIDENCE: HIGH. Original 0x14292cea0. The exec thunk reads the Payload array and runs the _Validate virtual (vtable +0x948, Stub_ReturnTrue) and then _Implementation (vtable +0x950, 0x141eae6f0), or RPC_ValidateFailed on failure.
// Original address: 0x14292cea0 (exec).
if (!ServerFireAIDirectorEventBatch_Validate(Payload))
{
	RPC_ValidateFailed(TEXT("ServerFireAIDirectorEventBatch_Validate"));
	return;
}
ServerFireAIDirectorEventBatch_Implementation(Payload);
