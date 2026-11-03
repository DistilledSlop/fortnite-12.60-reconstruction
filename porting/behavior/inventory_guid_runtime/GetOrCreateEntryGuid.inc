// CONFIDENCE: HIGH
if (!Entry.ItemGuid.IsValid())
{
    Entry.ItemGuid = FGuid::NewGuid();
}
return Entry.ItemGuid;
