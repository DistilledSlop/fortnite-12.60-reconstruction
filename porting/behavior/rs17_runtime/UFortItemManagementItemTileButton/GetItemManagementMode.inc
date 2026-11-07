// Original body address: 0x144f5d840 (thunk 0x145274420)
// Offset evidence: IsValid (0x142ee7880) on HostItemManagementScreen weak at +0xba8 (reflected); false -> XOR AL,AL; true -> Get (0x142ee5dd0) and byte +0x520 (Mode of UFortItemManagementScreen).
// CONFIDENCE: HIGH
if (!HostItemManagementScreen.IsValid())
{
    return static_cast<EFortItemManagementMode>(0);
}
return HostItemManagementScreen.Get()->Mode;
