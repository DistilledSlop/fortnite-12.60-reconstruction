// Behaviour snippet: AFortPlayerControllerAthena::GetMatchReport (body only; spliced into the generated member function).
// Original address: 0x1419d5220
// Evidence: Role (0xf0) compared to 2.
// Evidence: Qword [RCX+0x3318] is reflected MatchReport (Object).
// CONFIDENCE: HIGH
if (static_cast<uint8>(Role) != 2)
{
    return nullptr;
}
return MatchReport;
