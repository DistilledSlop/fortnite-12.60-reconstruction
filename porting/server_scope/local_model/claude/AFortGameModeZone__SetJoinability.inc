// CONFIDENCE: LOW
// Original addresses: exec thunk 0x1428f39c0 (reads bMatchJoinable, virtual slot 0xbe8 on this) -> impl 0x141eafcd0 (owner AFortGameModeZone, inherited by AFortGameModeAthena).
// Session plumbing (0x140fc7960, 0x2d8 vcall, 0x14217ac30, 0x142149480, 0x142908180) is captured but not ported in this pass.
UWorld* World = GetWorld();                                     // virtual 0x150
if (GameSession != nullptr)                                      // this+0x270
{
	// 0x142908180: notify on the game session (not ported).
	NotifyGameSessionJoinabilityChanged();
}
if (World == nullptr || FortGameSession == nullptr)              // this+0x3d8 (FortGameSession)
{
	return;
}

// Update the gatherable session entry for this match (name table ids 0x11f, literal "GATHERABLE").
// Session object from 0x140fc7960(World, 0); entry from vcall 0x2d8(name).
TSharedPtr<FFortGameSessionHelper> SessionHelper = GetSessionHelper(World);
if (SessionHelper.IsValid())
{
	FName GatherableId = FName_FromEName(0x11f);
	UObject* Entry = SessionHelper->FindOrCreateGatherableEntry(GatherableId);   // vcall 0x2d8
	if (Entry != nullptr)
	{
		FName GatherableName = FName(TEXT("GATHERABLE"));
		SetGatherableProperty(Entry, GatherableName, bMatchJoinable, 2);    // 0x140d5d310
		// Four joinability bytes on the entry (+0x10, +0x15, +0x17, +0x18) all take the same value.
		Entry->JoinableFlag_10 = bMatchJoinable;
		Entry->JoinableFlag_15 = bMatchJoinable;
		Entry->JoinableFlag_17 = bMatchJoinable;
		Entry->JoinableFlag_18 = bMatchJoinable;

		// Build the session update record (0x30 bytes, vtable DAT_145b415b8 then DAT_145b41610) and register it.
		FSessionUpdateRecord Record = BuildSessionUpdateRecord(this, Entry);   // FUN_142ee3b30, FUN_142bad740, LAB_141e8d238
		FortGameSession->RegisterGatherableUpdate(GatherableId, Entry, Record, 3);   // 0x14217ac30 on this+0x3d8
		FortGameSession->FlushGatherableUpdates();                             // 0x142149480
	}
}

// Mirror the flag onto the game state (this+0x278 GameState) when it is an AFortGameState (0x142927a60).
if (GameState != nullptr && GameState->IsA(AFortGameState::StaticClass()))   // 0x142927a60
{
	GameState->bJoinable_968 = bMatchJoinable;                   // +0x968
}
