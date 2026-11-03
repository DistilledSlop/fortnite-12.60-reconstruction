// UCommonInputManager: these members accompany actual tickable/held dispatch.
public:
    virtual void PostInitProperties() override;
private:
    friend class UCommonUISubsystem;
    TWeakObjectPtr<UCommonUISubsystem> NativeOwnerSubsystem; // original+d0
    TWeakObjectPtr<UCommonInputSubsystem> NativeInputSubsystem; // original+d8
