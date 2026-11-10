// CONFIDENCE: HIGH. Exec thunk: asm (0x142a8d89e CALL 0x141a4aa10) passes the context object through unchanged in RCX, so the body is a direct tail call with no argument reads. Returns the bool from the helper.
// Original address: 0x142a8d880 (exec). Helper 0x141a4aa10 (deps AFortPlayerStateAthena__IsInSquad__SquadHelper.json).
bool bInSquad = IsInSquadHelper();
return bInSquad;
