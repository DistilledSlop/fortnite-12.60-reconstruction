// Original native transition producers 14081c970, 14081cc60 and 14081aa90.
void NativeSetInputLock(FName Reason, ECommonInputType InputType, bool bEnable);
void NativeBroadcastInputMethodChanged();
TOptional<ECommonInputType> NativeInputTypeOverride;
virtual void Initialize(FSubsystemCollectionBase& Collection) override;
virtual void Deinitialize() override;
void NativeSetInputTypeFilter(ECommonInputType InputType, FName Reason, bool bBlock);
bool NativeTickPreprocessorOrder(float DeltaTime);
void NativeHandleShowKeysChanged(IConsoleVariable* Variable);
TSharedPtr<class FCommonInputPreprocessor> NativeInputPreprocessor;
FDelegateHandle NativePreprocessorTicker;
