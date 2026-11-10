// CONFIDENCE: MEDIUM
// Original: exec thunk 0x142a89960. Reflection params: PreviewPawn (Object, 0), DesiredActiveVariant (Struct, 1),
// VariantChannelToChange (Struct, 2), ItemTemplateToChange (Name, 3). The thunk reads ItemTemplateToChange first and PreviewPawn last,
// and calls (this, ItemTemplateToChange, VariantChannelToChange, DesiredActiveVariant, PreviewPawn).
// _Validate is virtual slot 0x6f8 which resolves to Stub_ReturnTrue 0x1406801d0 (STUB): it always returns true.
// _Implementation is virtual slot 0x700 which resolves to forwarder 0x1424276e0, which tail-calls virtual slot 0x750
// (ChangeActiveVariantForCosmeticItem body at 0x14240c7c0, see AFortPlayerState__ChangeActiveVariantForCosmeticItem.inc).
if (!ServerChangeActiveVariantForCosmeticItem_Validate(ItemTemplateToChange, VariantChannelToChange, DesiredActiveVariant, PreviewPawn))
{
    RPC_ValidateFailed(TEXT("ServerChangeActiveVariantForCosmeticItem_Validate"));
    return;
}
ServerChangeActiveVariantForCosmeticItem_Implementation(ItemTemplateToChange, VariantChannelToChange, DesiredActiveVariant, PreviewPawn);
