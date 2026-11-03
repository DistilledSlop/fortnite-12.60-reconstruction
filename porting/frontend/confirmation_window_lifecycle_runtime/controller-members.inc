public:
    UFUNCTION(BlueprintCallable, BlueprintPure)
    bool IsClientBot() const;
private:
    // Native original +0x1818 and +0x1838; producer writes remain unrecovered.
    bool bNativeClientBotFlag1818 = false;
    bool bNativeClientBotFlag1838 = false;
