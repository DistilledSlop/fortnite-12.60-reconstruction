// CONFIDENCE: HIGH
// Original 0x1417cdda0, called by AFortQuickBars::SwapItems for both swapped items.
// The original initializes all eleven function-local static FStrings before any test.
static const FString EventName(TEXT("QuickBars.SlotItem"));
static const FString ItemNameKey(TEXT("ItemName"));
static const FString ItemDisplayNameKey(TEXT("ItemDisplayName"));
static const FString ItemDurabilityKey(TEXT("ItemDurability"));
static const FString ItemLevelKey(TEXT("ItemLevel"));
static const FString PreviousSlotKey(TEXT("PreviousSlot"));
static const FString NewSlotKey(TEXT("NewSlot"));
static const FString ReplacedItemNameKey(TEXT("ReplacedItemName"));
static const FString ReplacedItemDisplayNameKey(TEXT("ReplacedItemDisplayName"));
static const FString ReplacedItemDurabilityKey(TEXT("ReplacedItemDurability"));
static const FString ReplacedItemLevelKey(TEXT("ReplacedItemLevel"));

if ((!Item && !ReplacedItem) || !Controller)
{
    return;
}
if (Controller->IsA(AFortPlayerControllerAthena::StaticClass()))
{
    return;
}

// Slot description helper (original 0x1417e67d0, only called here). Out starts empty.
auto BuildSlotLabel = [](EFortQuickBars InBar, int32 Slot, FString& Out)
{
    if (InBar == EFortQuickBars::Secondary)
    {
        Out = TEXT("F");
        Out.AppendInt(Slot + 1);
    }
    else if (InBar == EFortQuickBars::Creative)
    {
        Out = TEXT("C");
        Out.AppendInt(Slot + 1);
    }
    else if (InBar == EFortQuickBars::Primary)
    {
        switch (Slot)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            Out.AppendInt(Slot + 1);
            break;
        case 4:
            Out = TEXT("Z");
            break;
        case 5:
            Out = TEXT("X");
            break;
        case 6:
            Out = TEXT("C");
            break;
        case 7:
            Out.AppendInt(5);
            break;
        case 8:
            Out.AppendInt(6);
            break;
        default:
            Out = TEXT("NULL");
            break;
        }
    }
};

// The provider shared reference is copied (non-atomic +1) and released at scope exit.
TSharedPtr<FFortNativeAnalyticsProvider, ESPMode::NotThreadSafe> ProviderRef = Controller->NativeGetAnalyticsProvider();
FFortNativeAnalyticsProvider* Provider = ProviderRef.Get();
if (Provider)
{
    FString ItemNameStr;
    FString ItemDisplayStr;
    FString PreviousSlotStr;
    FString NewSlotStr;
    FString ReplacedNameStr;
    FString ReplacedDisplayStr;
    TArray<FAnalyticsEventAttribute> Attributes;
    Attributes.Reserve(12);

    if (Item)
    {
        // Native does not null-check the item definition before reading its FName.
        UFortItemDefinition* Definition = Item->GetItemDefinitionBP();
        ItemNameStr = Definition->GetFName().ToString();
        ItemDisplayStr = Item->GetDisplayName().ToString();
        Attributes.Emplace(ItemNameKey, ItemNameStr);
        Attributes.Emplace(ItemDisplayNameKey, ItemDisplayStr);
        Attributes.Emplace(ItemDurabilityKey, static_cast<double>(Item->GetNormalizedDurability()));
        Attributes.Emplace(ItemLevelKey, static_cast<double>(Item->GetLevel()));
        BuildSlotLabel(Bar, PreviousSlot, PreviousSlotStr);
        BuildSlotLabel(Bar, NewSlot, NewSlotStr);
        if (PreviousSlot > -1)
        {
            Attributes.Emplace(PreviousSlotKey, PreviousSlotStr);
        }
        Attributes.Emplace(NewSlotKey, NewSlotStr);
    }
    if (ReplacedItem)
    {
        UFortItemDefinition* Definition = ReplacedItem->GetItemDefinitionBP();
        ReplacedNameStr = Definition->GetFName().ToString();
        ReplacedDisplayStr = ReplacedItem->GetDisplayName().ToString();
        Attributes.Emplace(ReplacedItemNameKey, ReplacedNameStr);
        Attributes.Emplace(ReplacedItemDisplayNameKey, ReplacedDisplayStr);
        Attributes.Emplace(ReplacedItemDurabilityKey, static_cast<double>(ReplacedItem->GetNormalizedDurability()));
        Attributes.Emplace(ReplacedItemLevelKey, static_cast<double>(ReplacedItem->GetLevel()));
    }
    Provider->NativeRecordEvent(EventName, Attributes);
}
