// CONFIDENCE: HIGH - the validate target is the shared Stub_ReturnTrue, and the implementation is a tail forward to ChangeActiveVariantForCosmeticItem.
// Original address: 0x142a46cd0 (RPC exec thunk). Validate is virtual slot 0xfb8 -> 0x1406801d0 (Stub_ReturnTrue).
// Implementation is virtual slot 0xfc0 -> 0x14237bc60, which tail-jumps through virtual slot 0x1ba0 -> ChangeActiveVariantForCosmeticItem (0x14236ed00).
// Declaration order (inventory list reversed, confirmed by FFrame read order and property offsets 0/8/16/24):
//   ItemTemplateToChange (Name), VariantChannelToChange (struct), DesiredActiveVariant (struct), PreviewPawn (object).
if (!ForwardActiveVariantChangeRequest_Validate(ItemTemplateToChange, VariantChannelToChange, DesiredActiveVariant, PreviewPawn))
{
	RPC_ValidateFailed(TEXT("ForwardActiveVariantChangeRequest_Validate"));
	return;
}
ChangeActiveVariantForCosmeticItem(ItemTemplateToChange, VariantChannelToChange, DesiredActiveVariant, PreviewPawn);
