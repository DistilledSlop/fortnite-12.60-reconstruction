// Candidate module. The helper names below are source aliases.
#include "CommonUIModuleRuntime.h"
#include "UCommonUISettings.h"
#include "Engine/AssetManager.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"

class FCommonUIModule : public IModuleInterface
{
public:
    virtual void StartupModule() override
    {
        UClass* SettingsClass = UCommonUISettings::StaticClass();
        UObject* Outer = GetTransientPackage();
        Settings = NewObject<UCommonUISettings>(Outer, SettingsClass, NAME_None, RF_NoFlags);
        Settings->AddToRoot();
        if (Settings->bAutoLoadData)
        {
            Settings->NativeInitializeData();
        }
    }
    virtual void ShutdownModule() override
    {
        if (!GExitPurge) Settings->RemoveFromRoot();
        Settings = nullptr;
    }
    virtual FStreamableManager* NativeGetStreamableManager()
    {
        (void)UAssetManager::Get();
        return &UAssetManager::GetStreamableManager();
    }
    // Original +0x48 returns null, but its API type is unresolved; no replacement API is declared.
    virtual UCommonUISettings* NativeGetSettings() { return Settings; }
private:
    UCommonUISettings* Settings = nullptr;
};

UCommonUISettings* NativeGetCommonUISettings()
{
    FModuleManager& Manager = FModuleManager::Get();
    IModuleInterface* Module = IsInGameThread()
        ? Manager.LoadModule(TEXT("CommonUI"))
        : Manager.GetModule(TEXT("CommonUI"));
    return static_cast<FCommonUIModule*>(Module)->NativeGetSettings();
}

IMPLEMENT_MODULE(FCommonUIModule, CommonUI);
