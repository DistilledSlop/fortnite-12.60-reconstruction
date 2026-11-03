protected:
    virtual void NativeDestruct() override; // 1407d1b50
private:
    void NativeSetInputActionHandler(FDataTableRowHandle Row, FCommonPanelCommitNative Commit,
        FCommonActionBindingProgress Progress, UCommonPopupMenu* Popup);
    bool NativeIsHoldingInputAction(const FDataTableRowHandle& Row) const;
    void NativeNotifyInputActionsChanged();
    void NativeRemoveRoutedInputActionHandlers();
