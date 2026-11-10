// CONFIDENCE: MEDIUM - the PlayerState branch is fully read; the local-player branch calls helper 0x14237f580 (280 lines) that was not ported.
// Original address: 0x142a47360 (exec thunk, struct return). Helper: 0x14237f3d0. Copy-assign: 0x140d7c4c0.
// Local player branch: when Player is a UFortLocalPlayer, the account id comes from helper 0x14237f580 (not ported, see deps).
// Otherwise the id is a copy of PlayerState->UniqueId (shared-pointer copy: object at +0x250, controller at +0x258 with refcount increment).
// Otherwise the result is an empty id.
UFortLocalPlayer* LocalPlayer = Cast<UFortLocalPlayer>(Player);
if (LocalPlayer != nullptr)
{
	return GetLocalPlayerGameAccountId_0x14237f580(LocalPlayer);
}
if (PlayerState != nullptr)
{
	return FUniqueNetIdRepl(PlayerState->UniqueId);
}
return FUniqueNetIdRepl();
