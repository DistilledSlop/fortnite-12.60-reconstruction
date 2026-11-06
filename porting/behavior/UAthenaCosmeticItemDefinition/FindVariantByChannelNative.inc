// CONFIDENCE: high
// 0x141d507d0: search ItemVariants by nonempty channel tag.
for (UFortCosmeticVariant* Variant : ItemVariants)
{
    if (Channel.IsValid() && Variant->VariantChannelTag == Channel)
        return Variant;
}
return nullptr;
