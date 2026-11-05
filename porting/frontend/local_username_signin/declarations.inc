protected:
    UFUNCTION(BlueprintCallable)
    void StartSignIn();
    UFUNCTION(BlueprintCallable)
    void UpdateSignInButton(const FText& Text);
    UFUNCTION(BlueprintCallable)
    void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
public:
    // Local account selection is separate from the original online login state.
    UPROPERTY(BlueprintReadOnly, Category="Local Account")
    FString LocalSelectionStatus;
    UPROPERTY(BlueprintReadOnly, Category="Local Account")
    bool bLocalSelectionPending = false;
protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    UFUNCTION()
    void HandleLocalUsernameChanged(const FText& Text);
    UFUNCTION()
    void HandleLocalSignInClicked(UCommonButton* Button);
    UFUNCTION()
    void HandleLocalSelectionComplete(bool bSuccess, const FString& Error);
private:
    bool IsLocalUsernameValid() const;
    void UpdateLocalSignIn();
    void SubmitLocalUsername();
