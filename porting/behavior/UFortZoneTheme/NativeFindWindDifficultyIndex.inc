// CONFIDENCE: HIGH
// 0x1425f34d0: the first empty or equal-handle difficulty list wins.
for (int32 Index = 0; Index < DifficultyProperties.Num(); ++Index)
{
    const auto& Valid = DifficultyProperties[Index].ValidDifficulties;
    if (Valid.Num() == 0)
        return Index;
    for (const FDataTableRowHandle& Candidate : Valid)
        if (Candidate == Difficulty)
            return Index;
}
return INDEX_NONE;
