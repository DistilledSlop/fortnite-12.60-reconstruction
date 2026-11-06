// Original address: 0x1429c49f0; native body 0x14209e320 (exec thunk tail-jumps there after Stack.Code bump).
// Decompile source: porting/generated/decomp/AFortMinigame/OnRep_CurrentRound__1429c49f0.c
// Inferred: target reads CurrentRound (int32 at +0x42c) and passes {this, CurrentRound} as the two-parameter argument block of OnCurrentRoundChangedDelegate (this+0x3e8, FOnCurrentRoundChanged(AFortMinigame*, int32)).
// CONFIDENCE: HIGH
OnCurrentRoundChangedDelegate.Broadcast(this, CurrentRound);
