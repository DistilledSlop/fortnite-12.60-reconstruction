public:
    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool HasInputActionHandler(FDataTableRowHandle InputActionRow) const;
    UFUNCTION(BlueprintCallable)
    void SetInputActionHandler(FDataTableRowHandle InputActionRow, FCommonActionCommited CommitedEvent);
    UFUNCTION(BlueprintCallable)
    void SetInputActionHandlerWithPopupMenu(FDataTableRowHandle InputActionRow, FCommonActionCommited CommitedEvent, UCommonPopupMenu* PopupMenu);
    UFUNCTION(BlueprintCallable)
    void SetInputActionHandlerWithProgress(FDataTableRowHandle InputActionRow, FCommonActionCommited CommitedEvent, FCommonActionProgressSingle ProgressEvent);
    UFUNCTION(BlueprintCallable)
    void SetInputActionHandlerWithProgressPopupMenu(FDataTableRowHandle InputActionRow, FCommonActionCommited CommitedEvent, FCommonActionProgressSingle ProgressEvent, UCommonPopupMenu* PopupMenu);
    UFUNCTION(BlueprintCallable)
    void AddInputActionHandler(UDataTable* DataTable, FName RowName, FCommonActionCommited CommitedEvent);
    UFUNCTION(BlueprintCallable)
    void AddInputActionHandlerWithPopup(UDataTable* DataTable, FName RowName, FCommonActionCommited CommitedEvent, UCommonPopupMenu* PopupMenu);
    UFUNCTION(BlueprintCallable)
    void AddInputActionHandlerWithProgress(UDataTable* DataTable, FName RowName, FCommonActionCommited CommitedEvent, FCommonActionProgressSingle ProgressEvent);
    UFUNCTION(BlueprintCallable)
    void AddInputActionHandlerWithProgressPopup(UDataTable* DataTable, FName RowName, FCommonActionCommited CommitedEvent, FCommonActionProgressSingle ProgressEvent, UCommonPopupMenu* PopupMenu);
    UFUNCTION(BlueprintCallable)
    void AddInputActionNoHandler(UDataTable* DataTable, FName RowName);
    UFUNCTION(BlueprintCallable)
    void RemoveInputActionHandler(FDataTableRowHandle InputActionRow);
    UFUNCTION(BlueprintCallable)
    void RemoveAllInputActionHandlers();
