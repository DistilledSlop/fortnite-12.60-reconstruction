// CONFIDENCE: HIGH (dispatch) / MEDIUM (base body)
// Original: exec thunk 0x142a43700. FFrame read order: ItemTemplateToChange, VariantChannelToChange, DesiredActiveVariant,
// PreviewPawn. The thunk dispatches to the virtual at slot 0x1ba0 (base body 0x14236ed00, see deps
// AFortPlayerController__ChangeActiveVariantForCosmeticItemNative). The base body logs when a preview pawn is given
// (the pawn is unused) and forwards the request to the player state (+0x220) when it is an AFortPlayerState.
NativeChangeActiveVariantForCosmeticItem(ItemTemplateToChange, VariantChannelToChange, DesiredActiveVariant, PreviewPawn);
