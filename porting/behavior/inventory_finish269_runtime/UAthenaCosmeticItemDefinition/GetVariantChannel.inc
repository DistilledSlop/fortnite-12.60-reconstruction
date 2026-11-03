// Native0x141d507d0 CONFIDENCE: HIGH
// Exec 0x14271eb90; ItemVariants TArray at +0x418 (count +0x420); UFortCosmeticVariant::VariantChannelTag at +0x28; no null guard in original loop
for (UFortCosmeticVariant* Variant : ItemVariants)
{
	const bool bInputIsValid = InVariantChannelTag.IsValid();
	if (bInputIsValid && Variant->VariantChannelTag == InVariantChannelTag)
	{
		return Variant;
	}
}
return nullptr;
