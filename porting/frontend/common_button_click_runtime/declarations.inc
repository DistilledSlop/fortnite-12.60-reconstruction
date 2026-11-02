public:
    // One shared native multicast for all native producers. Original slot +0xad8.
    FSimpleMulticastDelegate NativeClicked;
protected:
    // Source aliases for original native virtual slots +0x4d0 and +0x508.
    virtual void NativeOnClicked();
    virtual bool NativeGetClickAnalytics(FString& OutName, FString& OutType, FString& OutContext);
    friend struct FCommonButtonBaseClickProbeAccess;
