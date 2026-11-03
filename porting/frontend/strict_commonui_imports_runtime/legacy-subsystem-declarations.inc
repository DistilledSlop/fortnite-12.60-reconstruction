public:
    // Original1407f7b90; caller passes selected FortUIManager.bUseLegacyInputSystem.
    void NativeSetLegacyInputEnabled(bool bEnabled);
private:
    void NativeHandleLegacyInputSuspensionChanged(bool bSuspended);
