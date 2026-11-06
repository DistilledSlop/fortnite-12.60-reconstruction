// CONFIDENCE: HIGH
int32 Separator = INDEX_NONE;
const TArray<TCHAR>& Characters = Item.TemplateId.GetCharArray();
for (int32 Index = 0; Index < Characters.Num(); ++Index)
{
    if (Characters[Index] == TEXT('.')) { Separator = Index; break; }
}
if (Separator == INDEX_NONE)
{
    for (int32 Index = 0; Index < Characters.Num(); ++Index)
    {
        if (Characters[Index] == TEXT(':')) { Separator = Index; break; }
    }
}
Item.AlternateDefinitionKey = Separator == INDEX_NONE ? Item.TemplateId : Item.TemplateId.Left(Separator);
