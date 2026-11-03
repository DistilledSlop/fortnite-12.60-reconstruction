// Exact producers/removal, not an executable legacy manager policy by themselves.
void UCommonGlobalInputHandler::NativeRegisterGlobalAction(UUserWidget* Owner, FDataTableRowHandle Row,
    int32 Priority, FCommonActionCommited Commit, FCommonActionCompleteSingle Complete,
    FCommonActionProgressSingle Progress)
{
    if (Row.IsNull() || !Owner) return;
    FCommonLegacyGlobalAction* Found = NativeGlobalActions.FindByPredicate([&](const FCommonLegacyGlobalAction& Entry) {
        return Entry.Row == Row && Entry.Owner.Get() == Owner;
    });
    if (!Found)
    {
        FCommonLegacyGlobalAction Entry;
        Entry.Row = Row;
        Entry.Owner = Owner;
        Entry.HandlerData.State = static_cast<EInputActionState>(0);
        const int32 Index = NativeGlobalActions.Add(Entry);
        Found = &NativeGlobalActions[Index];
    }
    Found->InputPriority = Priority;
    Found->Commit = FCommonPanelCommitNative::CreateLambda([Commit](bool& Pass) mutable { Commit.ExecuteIfBound(Pass); });
    // Both multicast additions remain additions on an existing row-owner entry.
    // They are intentionally not AddUnique, nor replacements of existing listeners.
    Found->Complete.AddLambda([Complete]() mutable { Complete.ExecuteIfBound(); });
    Found->Progress.AddLambda([Progress](float Value) mutable { Progress.ExecuteIfBound(Value); });
    NativeGlobalActions.Sort(&FCommonLegacyGlobalAction::Less);
}

void UCommonGlobalInputHandler::NativeRemoveGlobalAction(UUserWidget* Owner, FDataTableRowHandle Row)
{
    if (Row.IsNull() || !Owner) return;
    // 14078fb70 is stable all-match RemoveAll, comparing resolved weak owners.
    NativeGlobalActions.RemoveAll([&](const FCommonLegacyGlobalAction& Entry) {
        return Entry.Row == Row && Entry.Owner.Get() == Owner;
    });
}
