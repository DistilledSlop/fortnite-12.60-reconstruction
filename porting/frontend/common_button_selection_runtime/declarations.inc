public:
    // Shared multicast at original native slot +0xb50; callbacks receive selected state.
    DECLARE_MULTICAST_DELEGATE_OneParam(FNativeSelectedChanged, bool);
    FNativeSelectedChanged NativeSelectedChanged;
protected:
    // Source aliases for original virtual slots +0x4b0 and +0x4b8.
    virtual void NativeOnSelected(bool bBroadcast);
    virtual void NativeOnDeselected(bool bBroadcast);
