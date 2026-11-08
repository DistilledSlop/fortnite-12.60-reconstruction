// Helper 0x1444ac7a0 is the out-of-line FString APlayerState::GetPlayerName() const (member function: RCX = this, RDX = hidden sret).
// lookup_offset: APlayerState PlayerNamePrivate (FString) @0x2f8 (Data @0x2f8, Num @0x300); the byte at 0x222 tested with a
// signed compare is the bUseCustomPlayerNames bit (declared right before GetPlayerNameCustom in PlayerState.h);
// vtbl[0x680] is GetPlayerNameCustom (called with the sret). 0x140690ac0/0x1458092ed are the TArray reserve + memcpy of the FString copy,
// 0x140682ce0 move-constructs the result, 0x142bee710 frees the temporaries (all emitted by C++).
// Callers: GetPlayerNameSafe 0x141ef63b0 etc. pass (player state, &local FString) and use RAX = that pointer.
new (OutName) FString(PlayerStateObject->GetPlayerName());
return OutName;
// CONFIDENCE: MEDIUM
