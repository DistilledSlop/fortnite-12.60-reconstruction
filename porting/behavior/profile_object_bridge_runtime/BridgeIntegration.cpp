// Proposed member definitions; integrate alongside the declarations in BridgeIntegration.h.
#include "UMcpProfile.h"
#include "UMcpItemAware.h"
#include "McpProfileItem.h"
#include "UObject/UObjectGlobals.h"

UObject* UMcpProfile::CreateItemObject(UClass* ItemClass, FString ItemId, FString TemplateId)
{
    // The base implementation consumes both strings but does not use them.
    return NewObject<UObject>(this, ItemClass);
}

void UMcpProfile::UpdateItemObject(TSharedPtr<FMcpProfileItem, ESPMode::NotThreadSafe> Item, bool bForcePopulate)
{
    // The native caller supplies a valid shared entry; it does not check for null.
    Item->Profile = this;
    UClass* ItemClass = ItemClassRegistry.FindRef(Item->TemplateId);
    if (!ItemClass)
    {
        ItemClass = ItemClassRegistry.FindRef(Item->AlternateDefinitionKey);
    }
    if (!ItemClass)
    {
        Item->ItemObject = nullptr;
        return;
    }

    UObject* Object = Item->ItemObject;
    if (!Object || Object->GetClass() != ItemClass)
    {
        Object = CreateItemObject(ItemClass, Item->ItemId, Item->TemplateId);
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
            Aware->McpItem = Item;
            if (!Aware->PopulateFromMcpItem(Item.Get()))
            {
                Item->ItemObject = nullptr;
            }
        }
    }
}
