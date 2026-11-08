// Behaviour snippet: UFortLocalPlayer::GetClientSettings (body only; spliced into the generated member function).
// Original address: 0x14237e5e0 (thunk 0x14298b000)
// Evidence: ClientSettingsRecord (+0x320) null -> null. Otherwise calls NativeHelper141e32a40 (0x141e32a40, ported helper) with the record pointer as its first argument; if it returns false the result is null, else ClientSettingsRecord is returned. The asm passes the record pointer, not a UFortLocalPlayer, so the argument is reinterpret_cast to the declared parameter type.
// CONFIDENCE: MEDIUM
if (ClientSettingsRecord == nullptr)
{
    return nullptr;
}
if (!UFortLocalPlayer::NativeHelper141e32a40(reinterpret_cast<UFortLocalPlayer*>(ClientSettingsRecord)))
{
    return nullptr;
}
return ClientSettingsRecord;
