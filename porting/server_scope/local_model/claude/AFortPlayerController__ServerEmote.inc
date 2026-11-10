// CONFIDENCE: LOW. Original exec 0x142a4b240 (RPC thunk, one FName parameter AssetName).
// Validate is virtual slot 0xe40 -> Stub_ReturnTrue (0x1406801d0), so no validation runs.
// Implementation is virtual slot 0xe48 -> 0x1423ab040 (69 lines), inlined below.
// The implementation queues an asynchronous asset load through the asset manager (slot 0x308); the completion delegate target is not captured, so the effect after the load is not ported.
if (IsValidNameIndex_0x142ce3730(AssetName))
{
	FSoftObjectPath EmotePath = EmoteAssetPath_0x141f1ffc0(0x67);
	UFortAssetManager* AssetManager = GetFortAssetManager_0x141df62a0();
	FEmoteLoadDelegate Completion;
	Completion.Controller = this;
	Completion.EmotePath = EmotePath;
	Completion.AssetName = AssetName;
	Completion.Context = GetLoadContext_0x142bad740();
	AssetManager->RequestAsyncEmoteLoad_Slot0x308(EmotePath, Completion);
}
