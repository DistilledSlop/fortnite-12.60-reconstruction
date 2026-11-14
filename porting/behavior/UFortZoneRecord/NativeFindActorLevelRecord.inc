// CONFIDENCE: HIGH. Original 0x141fbf660; returns a borrowed SavedLevels entry.
if (!Actor || !Actor->GetLevel())
{
    if (OutIndex)
        *OutIndex = INDEX_NONE;
    return nullptr;
}
const FName LevelName = Actor->GetLevel()->GetFName();
for (int32 Index = 0; Index < SavedLevels.Num(); ++Index)
{
    if (SavedLevels[Index].PackageName == LevelName)
    {
        if (OutIndex)
            *OutIndex = Index;
        return &SavedLevels[Index];
    }
}
if (OutIndex)
    *OutIndex = INDEX_NONE;
return nullptr;
