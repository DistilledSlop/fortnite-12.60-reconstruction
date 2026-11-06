// CONFIDENCE: HIGH
// The original requires a valid item and consumes its shared reference by value.
Item->Profile = Owner;
UClass* ItemClass = Registry.FindRef(Item->TemplateId);
if (!ItemClass)
{
    ItemClass = Registry.FindRef(Item->AlternateDefinitionKey);
}
if (!ItemClass)
{
    Item->ItemObject = nullptr;
    return;
}

UObject* Object = Item->ItemObject;
if (!Object || Object->GetClass() != ItemClass)
{
    Object = Owner->CreateItemObject(ItemClass, Item->ItemId, Item->TemplateId);
    Item->ItemObject = Object;
}
else if (!bForcePopulate)
{
    return;
}

if (Object)
{
    if (IMcpItemAware* Aware = Cast<IMcpItemAware>(Object))
    {
        Aware->NativeMcpItem = Item;
        if (!Aware->NativePopulateFromMcpItem(Item.Get()))
        {
            UE_LOG(LogProfileSys, Error, TEXT("Failed PopulateFromMcpItem on item %s of type %s which was mapped to class %s"),
                *Item->ItemId, *Item->TemplateId, *ItemClass->GetName());
            Item->ItemObject = nullptr;
        }
    }
}
