public:
    virtual bool Initialize() override;
protected:
    // Original unreflected bytes +2a2/+2a3/+2a4; UObject allocation initializes to zero.
    TEnumAsByte<EButtonClickMethod::Type> NativeClickMethod = EButtonClickMethod::DownAndUp;
    TEnumAsByte<EButtonTouchMethod::Type> NativeTouchMethod = EButtonTouchMethod::DownAndUp;
    TEnumAsByte<EButtonPressMethod::Type> NativePressMethod = EButtonPressMethod::DownAndUp;
