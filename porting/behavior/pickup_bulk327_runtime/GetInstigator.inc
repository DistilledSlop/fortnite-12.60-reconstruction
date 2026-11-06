// CONFIDENCE: HIGH
{
    for (const FPickupInstigator& Entry : InstigatorRegistry.Items)
    {
        if (Entry.TrackedIndex == TrackedIndex)
        {
            if (&PickupInstigator != &Entry)
            {
                PickupInstigator.ReplicationID = INDEX_NONE;
                PickupInstigator.ReplicationKey = INDEX_NONE;
                PickupInstigator.MostRecentArrayReplicationKey = INDEX_NONE;
            }
            PickupInstigator.TrackedIndex = Entry.TrackedIndex;
            PickupInstigator.InstigatorData = Entry.InstigatorData;
            PickupInstigator.SpawnedPickups = Entry.SpawnedPickups;
            PickupInstigator.DroppedPickups = Entry.DroppedPickups;
            PickupInstigator.EventHandlerInstigator = Entry.EventHandlerInstigator;
            return true;
        }
    }
    return false;
}
