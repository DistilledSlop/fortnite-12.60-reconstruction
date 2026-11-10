// CONFIDENCE: MEDIUM. Original exec 0x1427c3370; the thunk reads EmotePrimaryAssetId (FPrimaryAssetId, by reference when
// the source is a variable) and forwards it to the native body 0x1416a66e0, kept as a direct helper call.
// That body resolves the emote asset through the world's asset registry, applies it to the bot, and logs
// "EmoteAsset %s was not correctly loaded. Is it properly cooked?" when the asset cannot be loaded.
PlayEmoteInternal(EmotePrimaryAssetId);
