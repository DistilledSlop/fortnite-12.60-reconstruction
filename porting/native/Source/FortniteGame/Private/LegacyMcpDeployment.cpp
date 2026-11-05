#include "LegacyMcpDeployment.h"

// Original resolver 0x1422cb480; recovery and external-context boundaries:
// porting/behavior/mcp_deployment_mapping_runtime/{evidence,default-evidence}.json.

namespace
{
struct FDeploymentEntry
{
    const TCHAR* Name;
    const TCHAR* SecondArgument;
    const TCHAR* ThirdArgument;
};

const FDeploymentEntry Entries[] = {
#include "LegacyMcpDeploymentTable.inl"
};

TCHAR FoldAscii(TCHAR Character)
{
    return Character >= TEXT('A') && Character <= TEXT('Z')
        ? Character + (TEXT('a') - TEXT('A')) : Character;
}

bool EqualAsciiInsensitive(const TCHAR* Left, const TCHAR* Right)
{
    while (*Left && *Right && FoldAscii(*Left) == FoldAscii(*Right))
    {
        ++Left;
        ++Right;
    }
    return FoldAscii(*Left) == FoldAscii(*Right);
}

bool StartsWithAsciiInsensitive(const TCHAR* Value, const TCHAR* Prefix)
{
    while (*Prefix)
    {
        if (!*Value || FoldAscii(*Value) != FoldAscii(*Prefix))
        {
            return false;
        }
        ++Value;
        ++Prefix;
    }
    return true;
}

bool ContainsAsciiInsensitive(const TCHAR* Value, const TCHAR* Needle)
{
    // All callers use nonempty ASCII needles. The original search stops at NUL.
    for (; *Value; ++Value)
    {
        if (StartsWithAsciiInsensitive(Value, Needle))
        {
            return true;
        }
    }
    return false;
}

FLegacyMcpDeploymentLookup LookupNormalized(const TCHAR* Normalized)
{
    for (const FDeploymentEntry& Entry : Entries)
    {
        if (EqualAsciiInsensitive(Normalized, Entry.Name))
        {
            return { ELegacyMcpDeploymentResult::Resolved, FString(Entry.SecondArgument), FString(Entry.ThirdArgument) };
        }
    }
    return { ELegacyMcpDeploymentResult::Unknown, FString(), FString() };
}

FString SelectDefault(ILegacyMcpDefaultContext& Context, ILegacyMcpDefaultDiagnosticSink& Sink,
    int32 Verbosity, bool& UE4WarningIssued)
{
    enum class EMode { Prod, Stage, GameDev };
    EMode Mode = EMode::Prod;
    void* Object = Context.ModeObject();
    if (Object)
    {
        if (Context.ModeValue(Object) == 1)
        {
            Mode = EMode::Stage;
        }
        else if (Context.ModeValue(Object) == 0)
        {
            Mode = EMode::GameDev;
        }
    }
    const FString Branch = Context.GetBranchAfterPlusToSlash();
    const bool IsUE4 = FCString::Strcmp(*Branch, TEXT("UE4")) == 0;
    const bool Tencent = Context.TencentFlag();
    const auto Found = [&Branch](const TCHAR* Needle)
    {
        return ContainsAsciiInsensitive(*Branch, Needle);
    };
    FString Name;
    if (Mode == EMode::Prod)
    {
        Name = !IsUE4 && !Found(TEXT("//Fortnite/Main")) && !Found(TEXT("//Fortnite/Dev"))
            ? TEXT("Fortnite") : TEXT("DevPlaytest");
    }
    else if (Mode == EMode::Stage)
    {
        Name = TEXT("CertStage");
    }
    else if (Found(TEXT("//Fortnite/Release-3.5.CN")))
    {
        Name = TEXT("FeatureALatest");
    }
    else if (!IsUE4 && !Found(TEXT("//Fortnite/Main")) && !Found(TEXT("//Fortnite/Dev")) &&
        !Found(TEXT("//UE4/Fortnite-Staging")))
    {
        if (Found(TEXT("//Fortnite/Release"))) Name = TEXT("ReleaseTesting");
        else if (Found(TEXT("//UE4/Release"))) Name = TEXT("UEReleaseTesting");
        else if (Found(TEXT("//UE4/"))) Name = TEXT("MainTesting");
        else if (Verbosity >= 2) Sink.Emit(ELegacyMcpDefaultDiagnostic::Undetermined, Name, Branch);
    }
    else if (Found(TEXT("//Fortnite/Dev-FeatureA"))) Name = TEXT("FeatureALatest");
    else if (Found(TEXT("//Fortnite/Dev-FeatureB")) || Tencent) Name = TEXT("FeatureBLatest");
    else if (Found(TEXT("//Fortnite/Dev-Athena"))) Name = TEXT("DevAthenaLatest");
    else Name = TEXT("DevLatest");

    if (IsUE4 && !UE4WarningIssued)
    {
        if (Verbosity >= 3) Sink.Emit(ELegacyMcpDefaultDiagnostic::Guessing, Name, Branch);
        UE4WarningIssued = true;
    }
    if (Verbosity >= 7) Sink.Emit(ELegacyMcpDefaultDiagnostic::UsingDefault, Name, Branch);
    return Name;
}
}

FLegacyMcpDeploymentLookup LookupLegacyMcpDeployment(const FString& Input)
{
    const TCHAR* Normalized = *Input;
    int32 NormalizedLength = Input.Len();
    const TCHAR* Prefix = TEXT("Fortnite");
    if (!EqualAsciiInsensitive(Normalized, Prefix) &&
        StartsWithAsciiInsensitive(Normalized, Prefix))
    {
        Normalized += 8;
        NormalizedLength -= 8;
    }
    if (NormalizedLength == 0)
    {
        return { ELegacyMcpDeploymentResult::RequiresDefault, FString(), FString() };
    }
    return LookupNormalized(Normalized);
}

FLegacyMcpDeploymentLookup ResolveLegacyMcpDeployment(
    const FString& Input, ILegacyMcpDefaultContext& Context,
    ILegacyMcpDefaultDiagnosticSink& Sink, int32 Verbosity, bool& UE4WarningIssued)
{
    FLegacyMcpDeploymentLookup Result = LookupLegacyMcpDeployment(Input);
    if (Result.Result != ELegacyMcpDeploymentResult::RequiresDefault)
    {
        return Result;
    }
    const FString Name = SelectDefault(Context, Sink, Verbosity, UE4WarningIssued);
    // The selected default enters the mapping directly, including when empty.
    return LookupNormalized(*Name);
}
