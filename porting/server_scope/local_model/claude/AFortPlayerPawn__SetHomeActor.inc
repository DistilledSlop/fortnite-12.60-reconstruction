// CONFIDENCE: MEDIUM; thunk 0x142a800f0 (NewActor read as one Object arg) calls impl 0x14234d380. Impl reads this->PlayerState (APawn::PlayerState, +0x238), checks it IsA AFortPlayerState (AFortPlayerState::StaticClass 0x142a85850), then calls the home-actor setter 0x142429b60 on it. Setter helpers 0x142944930/0x142944870/0x142942d20/0x142d6a820/0x142a75f10/0x143f7a920 and the vcalls it makes were captured but not read to depth two (see evidence notes).
AFortPlayerState* FortPlayerState = Cast<AFortPlayerState>(PlayerState);
if (FortPlayerState)
{
	FortPlayerState->SetHomeActorInternal(NewActor);
}
