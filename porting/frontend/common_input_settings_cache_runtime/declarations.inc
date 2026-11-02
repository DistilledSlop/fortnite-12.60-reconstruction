public:
    // Source alias for 14081b550 after the module supplies its settings instance.
    FCommonInputPlatformData NativeGetCurrentPlatformData();

protected:
    // Source aliases for original native entry points 14081d830 and 14081c1e0.
    void ValidateNativeInputDataCache();
    void InitializeNativeInputDataCache();
