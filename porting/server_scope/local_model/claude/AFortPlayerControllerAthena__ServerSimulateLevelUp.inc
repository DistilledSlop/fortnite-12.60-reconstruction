// CONFIDENCE: LOW for the tail (level-up reward broadcast), MEDIUM for the rest. Original exec 0x142a5c5f0 (RPC thunk, one int parameter ClientSimulatedLevel).
// Validate is virtual slot 0x2318 -> Stub_ReturnTrue (0x1406801d0), so no validation runs (STUB).
// Implementation is virtual slot 0x2320 -> 0x1419f2840, inlined below.
// FUN_141104100 is called when AthenaProfile is non-null, but it is a read-only query whose result the body discards. Omitted.
if (PlayerState != nullptr && PlayerState->IsA(AFortPlayerStateAthena::StaticClass()))
{
	PlayerState->SimulatedLevel = ClientSimulatedLevel;                 // FUN_141a5b580 writes PlayerState+0xe78
	PlayerState->ProcessLevelUpEvent(ClientSimulatedLevel);             // FUN_142a8b070: ProcessEvent with an int argument (UFunction name constant unresolved)
}
// Three reward/level-up records are built and discarded by the body. Side effects of FUN_1419b9230 were not read beyond its first 60 lines, so this part is LOW.
(void)BuildLevelUpRecord(this, GetSettings()->Field_0x1aa0, ClientSimulatedLevel); // tag at +0x1aa0
(void)BuildLevelUpRecord(this, GetSettings()->Field_0x1aa8, ClientSimulatedLevel); // tag at +0x1aa8
(void)BuildLevelUpRecord(this, GetSettings()->Field_0x1ab0, ClientSimulatedLevel); // tag at +0x1ab0
