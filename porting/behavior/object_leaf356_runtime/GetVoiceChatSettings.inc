// CONFIDENCE: high; native body and existing recovered fields reviewed.
if (!NativeRecoveredBool_0xd9)
{
	return ESelectableVoiceChatSetting::Off;
}
return NativeRecoveredBool_0xda ? ESelectableVoiceChatSetting::Everyone : ESelectableVoiceChatSetting::FriendsOnly;
