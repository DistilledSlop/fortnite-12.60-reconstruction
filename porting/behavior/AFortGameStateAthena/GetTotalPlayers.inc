// Behaviour snippet: AFortGameStateAthena::GetTotalPlayers (body only).
// Original address: 0x14291f680; native body 0x1419997b0.
// Decompile source: porting/generated/decomp/AFortGameStateAthena/GetTotalPlayers.c
// Evidence: porting/behavior/game_state_queries_evidence.md
// CONFIDENCE: HIGH
FTotalPlayers Result;
Result.Humans = TotalPlayers;
Result.Bots = TotalPlayersBots;
Result.Total = TotalPlayers + TotalPlayersBots;
return Result;
