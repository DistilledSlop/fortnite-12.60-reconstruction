#include "FortAttributeDefaultsPolicy.h"

#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Parse.h"

bool FortAttributeDefaultsPolicy::IsBROnly()
{
    static const bool bBROnly = []()
    {
        FString Run;
        FString TargetPlatforms;
        if (FParse::Value(FCommandLine::Get(), TEXT("run="), Run) &&
            Run.Equals(TEXT("cook"), ESearchCase::IgnoreCase) &&
            FParse::Value(FCommandLine::Get(), TEXT("targetplatform="), TargetPlatforms) &&
            (TargetPlatforms.Contains(TEXT("Android"), ESearchCase::IgnoreCase) ||
             TargetPlatforms.Contains(TEXT("iOS"), ESearchCase::IgnoreCase)))
        {
            return true;
        }

        bool bValue = false;
        GConfig->GetBool(TEXT("/Script/FortniteGame.FortGlobals"), TEXT("bBROnly"), bValue, GGameIni);
        if (FParse::Param(FCommandLine::Get(), TEXT("BROnly")))
        {
            bValue = true;
        }
        else if (FParse::Param(FCommandLine::Get(), TEXT("NoBROnly")))
        {
            bValue = false;
        }
        return bValue;
    }();
    return bBROnly;
}
