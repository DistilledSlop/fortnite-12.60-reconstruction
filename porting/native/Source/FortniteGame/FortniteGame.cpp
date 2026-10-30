#include "Engine/Texture2D.h"
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
        if (const UTexture2D* Texture = Cast<UTexture2D>(Asset))
        {
            const int64 FirstMipBytes = Texture->PlatformData && Texture->PlatformData->Mips.Num() > 0
                ? Texture->PlatformData->Mips[0].BulkData.GetBulkDataSize() : 0;
            UE_LOG(LogTemp, Display, TEXT("Fortnite texture probe: %dx%d, %d mips, first mip %lld bytes"),
                Texture->GetSizeX(), Texture->GetSizeY(), Texture->GetNumMips(), FirstMipBytes);
        }
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FFortniteGameModule, FortniteGame, "FortniteGame");
