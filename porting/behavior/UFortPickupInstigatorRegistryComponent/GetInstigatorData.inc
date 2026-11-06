// CONFIDENCE: HIGH
{
    for (const FPickupInstigator& Entry : InstigatorRegistry.Items)
    {
        if (Entry.TrackedIndex == TrackedIndex)
        {
            InstigatorData = Entry.InstigatorData;
            return true;
        }
    }
    return false;
}
