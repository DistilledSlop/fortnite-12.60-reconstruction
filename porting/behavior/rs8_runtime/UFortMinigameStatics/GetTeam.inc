// Original body address: 0x14246eb20
// Offset evidence: Game+0x278 (ASOLFortGame, no reflected member; NativeRecoveredUint64_0x278 holds minigame pointer) and Player+0x278 (ASOLFortPlayer; NativeRecoveredUint64_0x278 holds PlayerState). Tail-jumps to GetBucketForPlayerState.
// CONFIDENCE: HIGH
static_assert(sizeof(ASOLFortGame) > 0, "ASOLFortGame must be a complete type here");
if (Game == nullptr) { return 0; }
AFortMinigame* Minigame = reinterpret_cast<AFortMinigame*>(Game->NativeRecoveredUint64_0x278);
if (Minigame == nullptr || Player == nullptr) { return -1; }
APlayerState* PlayerState = reinterpret_cast<APlayerState*>(Player->NativeRecoveredUint64_0x278);
if (PlayerState == nullptr) { return -1; }
return Minigame->GetBucketForPlayerState(PlayerState);
