#pragma once

#include "CoreMinimal.h"

// Three adjacent original FString fields. The first field's semantic identity
// is unresolved; no embedded configuration or credentials are supplied here.
struct FLegacyMcpConfigurationRecord
{
    FString Key;
    FString Client;
    FString Secret;
};

// The incoming record must be owned outside Records. Searches Client, then Key,
// using the original ASCII-only case folding and NUL-terminated comparison.
// Returns the first matching index without changing either record; otherwise
// appends all three fields, clears Incoming, and returns the new index.
int32 AppendLegacyMcpConfiguration(TArray<FLegacyMcpConfigurationRecord>& Records,
    FLegacyMcpConfigurationRecord& Incoming);

struct ILegacyMcpConfigurationArguments
{
    virtual ~ILegacyMcpConfigurationArguments() = default;
    // Original argument getter 0x142caa880. Its complete parsing contract and
    // live command-line source remain the integration caller's responsibility.
    virtual bool ReadArgument(const TCHAR* Match, FString& Out, bool bStopOnSeparator) = 0;
};

// Only the paired successful override branch of original helper 0x1422c04b0.
// AuthSecret is queried only after AuthClient succeeds. A failed query leaves
// Records untouched. Does not execute embedded/fallback configuration branches.
bool ApplyLegacyMcpConfigurationOverrides(const FString& Key,
    ILegacyMcpConfigurationArguments& Arguments,
    TArray<FLegacyMcpConfigurationRecord>& Records);
