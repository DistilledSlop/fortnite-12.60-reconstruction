// CONFIDENCE: LOW
// Original address: 0x1423e9d60 (vtable slot 0x2090 of AFortPlayerControllerZone). Capture: captures/bde786e05f5242fa940c2fd929fb7571/0x1423e9d60.c
// Depth note: FUN_140b7ee30 (submit query), FUN_141eae290 (fallback join), FUN_142bad740 and FUN_140d12af0 are captured but were NOT read to depth two; their effects are described only.
UWorld* World = GetWorld();
AGameModeBase* GameMode = World != nullptr ? World->AuthorityGameMode : nullptr;   // World +0x128
if (GameMode == nullptr || !IsChildOfClass(GameMode, FUN_1428f1e50_Class))   // class check via FUN_1428f1e50 (name not recovered)
{
	return;
}
if (LogVerbosity > 4)   // DAT_1481348e0 > 4: logs bEnableDedicatedServerSocialBanVoiceQuery / ...Enforcement
{
	LogVoiceChatSocialBanOptions(UFortRuntimeOptions::GetDefault());
}
if (UFortRuntimeOptions::GetDefault()->bEnableDedicatedServerSocialBanVoiceQuery)   // runtime-options flag at +0xce
{
	FName QueryName = MakeFName(QuerySubsystemName);   // FUN_142cd4ab0 (name unrecovered)
	UObject* QuerySubsystem = GetSubsystemForName(GetWorld(), QueryName);   // FUN_140f67c80
	if (QuerySubsystem != nullptr)
	{
		uint8 QueryResult = GetVoiceChatQueryResult(QuerySubsystem);   // FUN_140d12af0 out value
		APlayerState* PS = PlayerState;   // controller +0x220
		if (QueryResult != 0 && PS != nullptr && IsA<AFortPlayerState>(PS))
		{
			// Builds a request object (vtable DAT_146149458 then DAT_1461494b0) holding a weak ref to this controller,
			// the controller pointer, a copy of ChannelName, and FUN_142bad740() (unread).
			FVoiceChatJoinRequest* Request = NewVoiceChatJoinRequest(this, ChannelName);
			FVoiceChatContext Context = BuildVoteRequestContext(PS);   // FUN_142419ef0 (same builder as the vote path)
			SubmitVoiceChatJoinRequest(QuerySubsystem, Context.UniqueId, Request);   // FUN_140b7ee30
			DestroyVoiceChatJoinRequest(Request);
			return;
		}
	}
}
// Fallback: query disabled, subsystem missing or PlayerState invalid. FUN_141eae290(GameMode, this, true, ChannelName).
FallbackVoiceChatJoin(GameMode, this, true, ChannelName);
