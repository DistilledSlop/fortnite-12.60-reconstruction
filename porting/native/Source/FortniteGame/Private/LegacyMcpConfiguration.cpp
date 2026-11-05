#include "LegacyMcpConfiguration.h"
#include "Misc/Parse.h"

// Original collection helpers 0x140d4b820 and 0x1422b28b0; comparator
// 0x142bcf780. See mcp_discovery_configuration_effects_runtime/evidence.json.
namespace
{
TCHAR FoldAscii(TCHAR Value)
{
    return Value >= TEXT('A') && Value <= TEXT('Z')
        ? Value + (TEXT('a') - TEXT('A')) : Value;
}

bool EqualOriginalString(const FString& Left, const FString& Right)
{
    const TCHAR* A = *Left;
    const TCHAR* B = *Right;
    while (*A == *B || FoldAscii(*A) == FoldAscii(*B))
    {
        if (!*A) return true;
        ++A; ++B;
    }
    return false;
}

class FExplicitMcpConfigurationArguments final : public ILegacyMcpConfigurationArguments
{
public:
    explicit FExplicitMcpConfigurationArguments(const TCHAR* InStream) : Stream(InStream) {}
    virtual bool ReadArgument(const TCHAR* Match, FString& Out, bool bStopOnSeparator) override
    {
        return FParse::Value(Stream, Match, Out, bStopOnSeparator);
    }
private:
    const TCHAR* Stream;
};
}

int32 AppendLegacyMcpConfiguration(TArray<FLegacyMcpConfigurationRecord>& Records,
    FLegacyMcpConfigurationRecord& Incoming)
{
    for (int32 Index = 0; Index < Records.Num(); ++Index)
    {
        if (EqualOriginalString(Records[Index].Client, Incoming.Client) &&
            EqualOriginalString(Records[Index].Key, Incoming.Key)) return Index;
    }
    const int32 Index = Records.Add(MoveTemp(Incoming));
    // Make source clearing explicit, independently of FString move details.
    Incoming.Key.Empty(); Incoming.Client.Empty(); Incoming.Secret.Empty();
    return Index;
}

bool ApplyLegacyMcpConfigurationOverrides(const FString& Key,
    ILegacyMcpConfigurationArguments& Arguments,
    TArray<FLegacyMcpConfigurationRecord>& Records)
{
    FLegacyMcpConfigurationRecord Incoming;
    if (!Arguments.ReadArgument(TEXT("AuthClient="), Incoming.Client, true) ||
        !Arguments.ReadArgument(TEXT("AuthSecret="), Incoming.Secret, true)) return false;
    // The original copies the global first field after both getters return.
    Incoming.Key = Key;
    AppendLegacyMcpConfiguration(Records, Incoming);
    return true;
}

bool ApplyLegacyMcpConfigurationOverrides(const FString& Key,
    const TCHAR* CommandLine, TArray<FLegacyMcpConfigurationRecord>& Records)
{
    FExplicitMcpConfigurationArguments Arguments(CommandLine);
    return ApplyLegacyMcpConfigurationOverrides(Key, Arguments, Records);
}
