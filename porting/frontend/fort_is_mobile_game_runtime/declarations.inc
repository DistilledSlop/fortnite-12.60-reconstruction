    // Original Win64 shipping contract: context is marshaled but not inspected.
    UFUNCTION(BlueprintPure, Category="Fortnite|Platform", meta=(WorldContext="WorldContextObject"))
    static bool IsMobileGame(const UObject* WorldContextObject);
