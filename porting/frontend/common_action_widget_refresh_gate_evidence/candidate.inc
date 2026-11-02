// Source-shaped candidate; place in the CommonInput implementation owner.
static int32 CommonInputShowKeys = 1;
static FAutoConsoleVariableRef CVarCommonInputShowKeys(
    TEXT("CommonInput.ShowKeys"), CommonInputShowKeys,
    TEXT("Should we show the keys for the current input device."), ECVF_Default);

// 14081d130 ignores the supplied subsystem; preserve dynamic console updates.
static bool ShouldShowInputKeys(const UCommonInputSubsystem* /*Subsystem*/)
{
    return CommonInputShowKeys != 0;
}
