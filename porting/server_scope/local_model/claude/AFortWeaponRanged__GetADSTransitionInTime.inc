// CONFIDENCE: LOW - the owned-locally branch (ADS transition time from WeaponData +0x240) is exact; the remote-owner branch and the 0x14269e3e0 gate are declared but not fully named.
// Original addresses: exec 0x142b4f480 -> native 0x1426c9f70.
if (!IsADSTransitionGateOpen())
{
    return 0.0f;
}
if (IsOwnedLocally())
{
    const UFortWeaponItemDefinition* Definition = GetWeaponData();
    return Definition ? Definition->ADSTransitionInTime : 0.0f;
}
return GetRemoteADSTransitionInTime();
