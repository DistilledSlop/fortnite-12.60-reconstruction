// CONFIDENCE: HIGH. Original 0x141f88c60 and typed FDataTableRowHandle
// resolver 0x141e88ca0 / UDataTable::FindRow<FGameDifficultyInfo> 0x1416b7770.
const TCHAR* Context = TEXT("AFortWorldManager::GetDifficultyRowForPIE");
const FDataTableRowHandle* Selected = &GameDifficultyRowPIEOverride;
if (!Selected->DataTable)
{
    if (!Selected->RowName.IsNone())
        UE_LOG(LogFortGameDifficulty, Log, TEXT("GameDifficultyRowPIEOverride data table is null even though the row is specifed (%s).  Will try to use global DifficultyRowForPIE instead."),
            *Selected->RowName.ToString());
}
else if (!Selected->RowName.IsNone())
{
    if (Selected->GetRow<FGameDifficultyInfo>(Context))
    {
        OutDifficulty = *Selected;
        return true;
    }
}
else
{
    UE_LOG(LogFortGameDifficulty, Log, TEXT("GameDifficultyRowPIEOverride data table is valid (%s), but no row is specifed.  Will try to use global DifficultyRowForPIE instead."),
        *Selected->DataTable->GetFName().ToString());
}
if (UFortGameInstance::NativeIsGameMode141ed8de0(this, 0))
{
    Selected = &UGameDataSTW::NativeGetGameDataSTW()->GameDifficultyRowPIE;
    if (Selected->GetRow<FGameDifficultyInfo>(Context))
    {
        OutDifficulty = *Selected;
        return true;
    }
}
return false;
