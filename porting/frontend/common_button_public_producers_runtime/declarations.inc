public:
    UFUNCTION(BlueprintCallable)
    void ClearSelection();
    UFUNCTION(BlueprintCallable)
    void SetIsSelected(bool InSelected, bool bGiveClickFeedback);
protected:
    UFUNCTION(BlueprintCallable)
    void SetSelectedInternal(bool bInSelected, bool bAllowSound, bool bBroadcast);
