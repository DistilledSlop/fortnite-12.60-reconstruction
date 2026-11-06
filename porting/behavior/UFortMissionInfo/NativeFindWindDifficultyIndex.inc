// CONFIDENCE: HIGH
// 0x1420b7ef0: the first empty or equal-handle difficulty list wins.
for (int32 Index = 0; Index < PerDifficultyProperties.Num(); ++Index)
{
    const auto& Valid = PerDifficultyProperties[Index].ValidDifficulties;
    if (Valid.Num() == 0)
        return Index;
    for (const FDataTableRowHandle& Candidate : Valid)
        if (Candidate == Difficulty)
            return Index;
}
return INDEX_NONE;
