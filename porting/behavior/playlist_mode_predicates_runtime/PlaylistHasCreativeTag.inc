// Original address: 0x141a49590.
// CONFIDENCE: HIGH
// Evidence: porting/behavior/playlist_mode_evidence.md
if (Playlist == nullptr)
{
	return false;
}
const FGameplayTag& ModeTag = FFortNativeGameplayTagsRuntime::Get().AthenaCreative();
return Playlist->GameplayTagContainer.HasTag(ModeTag);
