// CONFIDENCE: MEDIUM
// Original addresses: exec thunk 0x14283dc50 (virtual slot 0x778 -> 0x1426aad70, owner AFortDecoTool, i.e. the real body).
// Body from 0x1426aad70. Sub-helpers captured but only partly ported (see evidence notes).
bIsEquipped = false;
// 0x1426aafb0: unequip cleanup (385-line native helper, captured, NOT ported in this pass).
CleanupUnequipState();
// 0x143f84060(this, false): clears attached-visibility state when unequipping (partially read).
SetAttachedVisibility(false);
if (DecoHelper != nullptr)
{
	// virtual slot 0x640 on DecoHelper (class not resolved in package); called with no arguments.
	DecoHelper->OnDecoHelperUnequipped();
}
if (Role == ROLE_Authority)
{
	// virtual slot 0xb28 -> 0x142698610 (353-line native helper, captured, NOT ported in this pass).
	ReleaseDecoPlacementState();
	// virtual slot 0xae8 -> 0x1426b2930: SetDecoItemDefinitionInternal(nullptr, true).
	// It only acts when bIsEquipped is set, which was cleared above, so it is a no-op here.
	SetDecoItemDefinitionInternal(nullptr, true);
}
AFortPlayerController* InstigatorPC = GetInstigatorController();
if (InstigatorPC != nullptr && InstigatorPC->BuildPreviewMarker != nullptr)
{
	InstigatorPC->BuildPreviewMarker->SetActorHiddenInGame(true);
}
