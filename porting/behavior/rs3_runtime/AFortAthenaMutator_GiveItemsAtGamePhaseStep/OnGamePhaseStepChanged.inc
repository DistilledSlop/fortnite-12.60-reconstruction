// Original body address: 0x141adddf0 (thunk 0x142809540)
// Offset evidence: SafeZoneInterface (RDX) ObjectPointer null test (CMP qword [RDX],0). The non-null path tail-jumps (JNZ 0x14290f690)
// to the candidate AFortGameStateAthena::StaticClass() getter with the same arguments; the UClass result is discarded by the void return.
// GamePhaseStep and this are not otherwise used.
// CONFIDENCE: MEDIUM
if (SafeZoneInterface.GetObject() != nullptr)
{
    AFortGameStateAthena::StaticClass();
}
