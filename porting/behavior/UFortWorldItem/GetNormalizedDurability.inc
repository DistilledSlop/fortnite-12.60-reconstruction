// UFortWorldItem virtual override 0x14205a980.
// CONFIDENCE: HIGH
const UFortWorldItemDefinition* Definition = Cast<UFortWorldItemDefinition>(GetItemDefinitionBP());
if (!Definition || !Definition->bItemHasDurability)
{
    return 1.0f;
}
const float Durability = GetDurability();
const int32 Level = GetLevel();
const float Maximum = Definition->GetMaxDurability(Level);
const float Value = Durability / Maximum;
if (FMath::IsNaN(Value) || Value < 0.0f)
{
    return 0.0f;
}
return Value > 1.0f ? 1.0f : Value;
