#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"

class FFortniteGameModule : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override
    {
        FDefaultGameModuleImpl::StartupModule();
        FCoreDelegates::OnPostEngineInit.AddStatic(&FFortniteGameModule::ProbeAsset);
    }

private:
    static void ProbeAsset()
    {
        FString AssetPath;
        if (!FParse::Value(FCommandLine::Get(), TEXT("FortniteProbeAsset="), AssetPath))
        {
            return;
        }

        UObject* Asset = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
        UE_LOG(LogTemp, Display, TEXT("Fortnite asset probe: %s -> %s"),
            *AssetPath, Asset ? *Asset->GetClass()->GetName() : TEXT("LOAD_FAILED"));
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FFortniteGameModule, FortniteGame, "FortniteGame");
