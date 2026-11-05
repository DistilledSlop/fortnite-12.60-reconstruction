#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyLocalVFAdapter.h"

// Includes Material inventory entries, whose Contract is intentionally empty:
// their contract must be built from the actual factory expression metadata.
struct FLegacyOriginalUniformContractBinding
{
    uint32 ShaderIndex = 0;
    FString ParameterName;
    uint16 SourceRegister = 0;
    uint32 SourceResourceLayoutHash = 0;
    FLegacyLocalUniformContract Contract;
};

// Validates the entire pinned bundle and returns all occurrences by archive index.
// Out is unchanged on failure. No provider or bindings are installed.
bool LoadLegacyOriginalUniformBindings(const FString& Path, const FString& ExpectedSHA256,
    TArray<FLegacyOriginalUniformContractBinding>& Out, FString& Error);

/** Load one original ABI contract from caller-pinned exporter bytes. The caller
 * supplies the selected original shader occurrence, register and recorded hash;
 * the file digest authenticates provenance only to the caller's chosen pin.
 * Material requires a separate expression contract and is unsupported. The
 * returned metadata is registered current metadata; member trees are original.
 * No binding or renderer compatibility is established. Out is unchanged on failure.
 */
bool LoadLegacyOriginalUniformContract(const FString& Path, const FString& ExpectedSHA256,
    uint32 ShaderIndex, const FString& ParameterName, uint16 SourceRegister,
    uint32 SourceResourceLayoutHash, FLegacyLocalUniformContract& Out, FString& Error);
#endif
