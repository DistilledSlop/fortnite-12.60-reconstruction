#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyLocalVFAdapter.h"

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
