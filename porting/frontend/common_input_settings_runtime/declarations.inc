public:
    UCommonInputSettings(const FObjectInitializer& ObjectInitializer);

protected:
    // Original unreflected cache-valid state, shared by subsequent cache preparation.
    bool NativeInputDataLoaded;
