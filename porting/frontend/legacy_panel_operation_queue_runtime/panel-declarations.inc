protected:
    virtual void NativeAddedToActivationStack(); // original +4E8
    virtual void NativeRemovedFromActivationStack(); // original +4F0
private:
    bool bNativeInActivationStack = false; // original +304
    void NativeResetPanelOperations(); // original 1407f1b20
