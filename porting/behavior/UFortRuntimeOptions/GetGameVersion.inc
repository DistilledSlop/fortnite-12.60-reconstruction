// Behaviour snippet: UFortRuntimeOptions::GetGameVersion (body only).
// Original address: 0x142acdef0
// The thunk calls 0x1426ff110; dependencies and string behavior are in frontend-native-evidence.
// CONFIDENCE: HIGH
FString Build = FApp::GetBuildVersion();
FString Branch = FApp::GetBranchName();
FString Result;
if (Branch == TEXT("UE4"))
{
    Result = TEXT("Dev Build ");
}
else
{
    FString Suffix;
    FString Version;
    int32 Plus = Build.Find(TEXT("+"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
    if (Plus != INDEX_NONE)
    {
        Suffix = Build.Mid(Plus + 1);
    }
    int32 Changelist = Suffix.Find(TEXT("-CL-"), ESearchCase::IgnoreCase, ESearchDir::FromStart);
    if (Changelist != INDEX_NONE)
    {
        Version = Suffix.Left(Changelist);
    }
    Version = Version.Replace(TEXT("Release-"), TEXT(""), ESearchCase::IgnoreCase);
    if (Version.IsEmpty())
    {
        Version = FApp::GetBranchName();
    }
    Result = FString::Printf(TEXT("v%s"), *Version);
}
Result += FString::Printf(TEXT(" (%d)"), FEngineVersion::Current().GetChangelist());
return Result;
