// Behaviour snippet: UFortMissionActivationInfo::GetArbitratorType (body only).
// Original body address: 0x144f5be80 (thunk 0x1452896d0). this_delta_total 0.
// Offset evidence: [RCX+0xa8] = reflected member GameStateZone (object offset 0xa8). The body zero-initialises a byte-sized local at [RSP+0x30] (read back with MOVZX), calls GameStateZone->GetVoteArbitratorType(VoteType, &local) (callee 0x141e9fa30; declared const with signature (EFortVoteType, EFortVoteArbitratorType&) in AFortGameStateZone.h) when GameStateZone is non-null, ignores the callee's bool result, and returns the local. Null GameStateZone returns 0.
// CONFIDENCE: HIGH
EFortVoteArbitratorType ArbitratorType = static_cast<EFortVoteArbitratorType>(0);
if (GameStateZone != nullptr)
{
    GameStateZone->GetVoteArbitratorType(VoteType, ArbitratorType);
}
return ArbitratorType;
