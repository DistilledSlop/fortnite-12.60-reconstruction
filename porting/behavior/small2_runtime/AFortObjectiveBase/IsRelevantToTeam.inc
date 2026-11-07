// Behaviour snippet: AFortObjectiveBase::IsRelevantToTeam (body only; spliced into the generated member function).
// Original address: 0x14213d020
// Evidence: Byte [RCX+0x2a5] tested: reflected bitfield bRelevantToSpecificTeam (offset 677, mask 1).
// Evidence: Byte [RCX+0x2a6] compared with Team: reflected RelevantTeam (Byte).
// CONFIDENCE: HIGH
if (bRelevantToSpecificTeam && Team != static_cast<uint8>(RelevantTeam))
{
    return false;
}
return true;
