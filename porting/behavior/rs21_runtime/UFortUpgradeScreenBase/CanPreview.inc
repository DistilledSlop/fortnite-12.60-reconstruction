// Original body 0x145110d10 (thunk 0x14531e520). Offset evidence: Details +0x428 (reflected UFortUpgradeDetailsBase*); UpgradeInfo +0x260 (reflected UFortUpgradeInfo*); then UFortUpgradeInfo gap members bool +0x66 (must be zero) and Int32 +0x58 < Int32 +0x5c.
// CONFIDENCE: HIGH
UFortUpgradeInfo* Info = Details->UpgradeInfo;
if (Info == nullptr)
{
    return false;
}
if (Info->NativeRecoveredBool_0x66)
{
    return false;
}
return Info->NativeRecoveredInt32_0x58 < Info->NativeRecoveredInt32_0x5c;
