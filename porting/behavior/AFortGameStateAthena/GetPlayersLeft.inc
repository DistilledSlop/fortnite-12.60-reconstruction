// Behaviour snippet: AFortGameStateAthena::GetPlayersLeft (body only).
// Original address: 0x14291f1d0; native body 0x1419964d0.
// Decompile source: porting/generated/decomp/AFortGameStateAthena/GetPlayersLeft.c
// Evidence: porting/behavior/game_state_queries_evidence.md
// CONFIDENCE: HIGH
FPlayersLeft Result;
Result.Humans = PlayersLeft;
Result.Bots = PlayerBotsLeft;
Result.Total = PlayersLeft + PlayerBotsLeft;
return Result;
