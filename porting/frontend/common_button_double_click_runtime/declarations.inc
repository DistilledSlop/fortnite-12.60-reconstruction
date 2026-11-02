public:
    // Shared native multicast for original slot +0xaf0.
    FSimpleMulticastDelegate NativeDoubleClicked;
protected:
    // Source alias for original virtual slot +0x4d8.
    virtual void NativeOnDoubleClicked();
    // Source alias for the native FReply delegate receiver at 0x1407c6b10.
    FReply NativeHandleDoubleClicked();
