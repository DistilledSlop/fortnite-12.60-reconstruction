// Integration fragment: declarations belong on FFortAssets; include Misc/ScopeLock.h.
// virtual FString GetReferencerName() const override;
// virtual bool GetReferencerPropertyName(UObject* Object, FString& OutPropertyName) const override;

FString FFortAssets::GetReferencerName() const
{
    return TEXT("FFortAssets");
}

bool FFortAssets::GetReferencerPropertyName(UObject* Object, FString& OutPropertyName) const
{
    if (!Object)
    {
        return false;
    }

    {
        FScopeLock Lock(const_cast<FCriticalSection*>(&CriticalSection));
        if (AlwaysLoadedAssets.Contains(Object))
        {
            OutPropertyName = TEXT("AlwaysLoadedAssets");
            return true;
        }
        if (PostStartupAssets.Contains(Object))
        {
            OutPropertyName = TEXT("LoadedAssets");
            return true;
        }
    }

    const FName* Name = TemporaryReferences.Find(Object);
    if (!Name || *Name == NAME_None)
    {
        return false;
    }
    OutPropertyName = TEXT("GCPreventionPool.") + Name->ToString();
    return true;
}
