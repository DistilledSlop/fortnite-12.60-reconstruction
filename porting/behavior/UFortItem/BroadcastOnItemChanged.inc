// CONFIDENCE: HIGH
{
    if (bLog)
    {
        UE_LOG(LogFort, Log, TEXT("AFortInventory::SendInventoryChangedNotification UFortItem::BroadcastOnItemChanged calling OnItemChanged_Native.Broadcast"));
    }
    OnItemChanged_Native.Broadcast(bItemChanged, bAmmoChanged, bIngredientsChanged);
    if (bLog)
    {
        UE_LOG(LogFort, Log, TEXT("AFortInventory::SendInventoryChangedNotification UFortItem::BroadcastOnItemChanged calling OnItemChanged.Broadcast"));
    }
    OnItemChanged.Broadcast(bItemChanged, bAmmoChanged, bIngredientsChanged);
}
