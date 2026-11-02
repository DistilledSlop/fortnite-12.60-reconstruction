// The original +390 delegate is a single-cast UObject-bound delegate.
DECLARE_DELEGATE_FiveParams(FFortRouteTouchDelegate, int32, uint32,
    ETouchType::Type, const FVector2D&, FReply&);
public:
    FFortRouteTouchDelegate NativeRouteTouchDelegate;
    virtual bool InputTouch(FViewport* Viewport, int32 ControllerId, uint32 Handle,
        ETouchType::Type Type, const FVector2D& TouchLocation, float Force,
        FDateTime DeviceTimestamp, uint32 TouchpadIndex) override;
