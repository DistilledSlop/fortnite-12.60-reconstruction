// helper 0x141f881f0 (host UFortKismetLibrary): vtbl+0x150 call on Context (GetWorld), World+0x130 = GameState,
// Cast<AFortGameState>(GameState) via StaticClass IsA fast path, then int32 at +0x2f4 (AFortGameState::WorldLevel); otherwise 0.
// evidence: insns CALL [RAX+0x150]; MOV RBX,[RAX+0x130]; CALL AFortGameState::StaticClass; IsChildOf check; MOV EAX,[RBX+0x2f4]
// vtable slot 0x150 is taken to be UObject::GetWorld (inferred from the call pattern, not from a symbol)
// CONFIDENCE: MEDIUM
UWorld* World = Context->GetWorld();
if (!World)
{
    return 0;
}
AGameStateBase* GameStateBase = World->GameState;
if (!GameStateBase)
{
    return 0;
}
AFortGameState* FortGameState = Cast<AFortGameState>(GameStateBase);
if (!FortGameState)
{
    return 0;
}
return FortGameState->WorldLevel;
