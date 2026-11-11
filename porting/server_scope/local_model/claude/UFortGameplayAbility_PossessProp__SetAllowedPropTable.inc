// CONFIDENCE: MEDIUM. Original exec 0x1428fe8e0; core 0x141638dc0.
// Only a changed, non-null table rebuilds AllowedPropClasses from the table rows; clearing the table keeps the old list.
if (InAllowedPropTable == AllowedPropTable)
{
	return;
}

AllowedPropTable = InAllowedPropTable;
if (!IsValid(InAllowedPropTable))
{
	return;
}

TArray<FAllowedPropTableRow*> Rows;
InAllowedPropTable->GetAllRows(TEXT("UFortGameplayAbility_PossessProp::SetAllowedPropTable"), Rows);

AllowedPropClasses.Reset();
AllowedPropClasses.Reserve(Rows.Num());
for (const FAllowedPropTableRow* Row : Rows)
{
	AllowedPropClasses.Add(Row->AllowedPropClass);
}
