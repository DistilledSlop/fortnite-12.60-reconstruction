protected:
    virtual void NativeAddedToActivationStack(); // original +4E8
    virtual void NativeRemovedFromActivationStack(); // original +4F0
    virtual void NativePopSelfFromInputManager1407e1130(); // original +4C0
    bool NativeIsInActivationStack304() const { return bNativeInActivationStack; }
private:
    bool bNativeInActivationStack = false; // original +304
    void NativeResetPanelOperations(); // original 1407f1b20
