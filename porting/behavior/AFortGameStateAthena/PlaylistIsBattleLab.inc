// Original address: 0x141a4a300.
// CONFIDENCE: HIGH
// Evidence: porting/behavior/playlist_mode_evidence.md and native_gameplay_tags_runtime/README.md
if (Playlist == nullptr)
{
	return false;
}
const FGameplayTag& ModeTag = FFortNativeGameplayTagsRuntime::Get().AthenaBattleLab();
return Playlist->GameplayTagContainer.HasTag(ModeTag);
