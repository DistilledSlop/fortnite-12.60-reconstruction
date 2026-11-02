// Included in the UFortUIDataConfiguration implementation translation unit.
// See RECOVERY.md for the two deliberately unresolved engine bindings.
static bool NativeIsBROnly141ed8a30()
{
    static int32 Cached = -1;
    if (Cached == -1)
    {
        FString Run, Target;
        bool bBROnly = false;
        if (FParse::Value(FCommandLine::Get(), TEXT("run="), Run, true)
            && Run.Equals(TEXT("cook"), ESearchCase::IgnoreCase)
            && FParse::Value(FCommandLine::Get(), TEXT("targetplatform="), Target, true)
            && (Target.Contains(TEXT("Android")) || Target.Contains(TEXT("iOS"))))
        {
            bBROnly = true;
        }
        else
        {
            GConfig->GetBool(TEXT("/Script/FortniteGame.FortGlobals"), TEXT("bBROnly"),
                bBROnly, NativeFortGlobalsConfigFilename141ed8a30());
            if (FParse::Param(FCommandLine::Get(), TEXT("BROnly")))
                bBROnly = true;
            else if (FParse::Param(FCommandLine::Get(), TEXT("NoBROnly")))
                bBROnly = false;
        }
        Cached = bBROnly ? 1 : 0;
    }
    return Cached == 1;
}

UFortUIDataConfiguration* UFortUIDataConfiguration::NativeGet144db86e0()
{
    static TWeakObjectPtr<UFortUIDataConfiguration> Cached;
    if (!Cached.IsValid())
    {
        const UFortUIDataConfiguration* Defaults = GetDefault<UFortUIDataConfiguration>();
        FString Run, Target;
        const bool bSwitchCook = FParse::Value(FCommandLine::Get(), TEXT("run="), Run, true)
            && Run.Equals(TEXT("cook"), ESearchCase::IgnoreCase)
            && FParse::Value(FCommandLine::Get(), TEXT("targetplatform="), Target, true)
            && Target.Contains(TEXT("Switch"));
        FString Path = bSwitchCook && NativeIsBROnly141ed8a30()
            ? Defaults->ErebusUIDataConfigurationPath
            : NativeIsBROnly141ed8a30() ? Defaults->BROnlyUIDataConfigurationPath
                                       : Defaults->DefaultUIDataConfigurationPath;
        UClass* Loaded = static_cast<UClass*>(StaticLoadObject(UClass::StaticClass(), nullptr,
            *Path, nullptr, LOAD_None, nullptr, true));
        if (Loaded)
        {
            Loaded->AddToRoot();
            Cached = static_cast<UFortUIDataConfiguration*>(Loaded->GetDefaultObject());
        }
        if (!Cached.IsValid())
        {
            NativeLogMissingUIConfiguration144db86e0();
            Cached = NewObject<UFortUIDataConfiguration>(GetTransientPackage());
            Cached.Get()->AddToRoot();
        }
        // The original tests a global byte followed by 1406801e0. The latter
        // returns false unconditionally in this image; hence its three extra
        // class loads cannot execute in this build.
    }
    return Cached.Get();
}
