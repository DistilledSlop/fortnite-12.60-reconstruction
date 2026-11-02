#include "CommonInputModuleRuntime.h"
#include "UCommonInputSettings.h"
#include "Modules/ModuleManager.h"
#include "HAL/IConsoleManager.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY(LogCommonInput);

namespace
{
int32 CommonInputShowKeys = 1;
FAutoConsoleVariableRef CVarCommonInputShowKeys(
    TEXT("CommonInput.ShowKeys"), CommonInputShowKeys,
    TEXT("Should we show the keys for the current input device."), ECVF_Default);
}

bool ShouldShowCommonInputKeys()
{
    return CommonInputShowKeys != 0;
}

class FCommonInputModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        UClass* SettingsClass = UCommonInputSettings::StaticClass();
        UObject* Outer = GetTransientPackage();
        Settings = NewObject<UCommonInputSettings>(Outer, SettingsClass, NAME_None, RF_NoFlags);
        Settings->AddToRoot();
    }

    virtual void ShutdownModule() override
    {
        if (!GExitPurge)
        {
            Settings->RemoveFromRoot();
        }
        Settings = nullptr;
    }

    virtual UCommonInputSettings* GetSettings() { return Settings; }

private:
    UCommonInputSettings* Settings = nullptr;
};

UCommonInputSettings* GetCommonInputSettings()
{
    FModuleManager& Manager = FModuleManager::Get();
    IModuleInterface* Module = IsInGameThread()
        ? Manager.LoadModule(TEXT("CommonInput"))
        : Manager.GetModule(TEXT("CommonInput"));
    return static_cast<FCommonInputModule*>(Module)->GetSettings();
}

IMPLEMENT_MODULE(FCommonInputModule, CommonInput);
