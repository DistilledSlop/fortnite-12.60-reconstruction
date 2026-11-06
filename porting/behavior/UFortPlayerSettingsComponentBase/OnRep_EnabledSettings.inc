// CONFIDENCE: high
// Original address: 0x142815100
// 0x142815100. Preserve the native FName lookup cache rebuilt by replication.
NativeEnabledSettingNames163.Reset();
for (const FString& Setting : EnabledSettings)
{
    NativeEnabledSettingNames163.Add(FName(*Setting));
}
