// Original body 0x144cd7c80 (thunk 0x14524a390). Offset evidence: qword [+0x20] is UObject::OuterPrivate (inherited; the owning ULocalPlayer); IsA(ULocalPlayer) and IsA(UFortLocalPlayer) gates on it; FortOnlineAccount (+0x288, reflected on UFortLocalPlayer) must be non-null; returns byte +0x7e8 of that UFortOnlineAccount (NativeRecoveredBool_0x7e8).
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
UObject* const Outer = *reinterpret_cast<UObject* const*>(Raw + 0x20);
if (!Outer)
{
    return false;
}
if (!Outer->IsA(ULocalPlayer::StaticClass()))
{
    return false;
}
if (!Outer->IsA(UFortLocalPlayer::StaticClass()))
{
    return false;
}
const UFortLocalPlayer* const FortLocalPlayer = static_cast<const UFortLocalPlayer*>(Outer);
UFortOnlineAccount* const Account = FortLocalPlayer->FortOnlineAccount;
if (!Account)
{
    return false;
}
return Account->NativeRecoveredBool_0x7e8;
