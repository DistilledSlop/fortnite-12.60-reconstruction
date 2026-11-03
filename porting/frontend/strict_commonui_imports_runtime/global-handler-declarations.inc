public:
    void NativeRegisterGlobalAction(UUserWidget* Owner, FDataTableRowHandle Row, int32 Priority,
        FCommonActionCommited Commit, FCommonActionCompleteSingle Complete, FCommonActionProgressSingle Progress);
    void NativeRemoveGlobalAction(UUserWidget* Owner, FDataTableRowHandle Row);
private:
    friend class UCommonUISubsystem;
    TArray<FCommonLegacyGlobalAction> NativeGlobalActions; // original+50
    TWeakObjectPtr<UCommonInputSubsystem> NativeInputSubsystem; // original+60
