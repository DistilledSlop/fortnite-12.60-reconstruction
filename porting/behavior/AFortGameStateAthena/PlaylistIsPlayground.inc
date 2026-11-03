// Original address: 0x141a4b0d0.
// CONFIDENCE: HIGH
// Evidence: porting/behavior/playlist_mode_evidence.md
if (Playlist == nullptr)
{
	return false;
}
const FGameplayTag& ModeTag = FFortNativeGameplayTagsRuntime::Get().AthenaPlayground();
return Playlist->GameplayTagContainer.HasTag(ModeTag);
