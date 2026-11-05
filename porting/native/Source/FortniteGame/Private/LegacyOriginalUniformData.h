#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyLocalVFAdapter.h"

/** Explicit replacement for one COMPLETE original non-struct member. Path starts
 * with ParameterName, uses '.' between members and '[N]' for struct-array
 * elements. Numeric/resource arrays are one member, without a trailing index.
 * Type must exactly describe the original leaf (including its relative Offset).
 * Bytes contains all elements, tightly packed, with no ABI array padding.
 * Resource bytes are native pointer-slot bytes, not serialized object IDs.
 */
struct FLegacyOriginalUniformMemberValue
{
    FString Path;
    FLegacyLocalMemberContract Type;
    TArray<uint8> Bytes;
};

/** Reconstruct original data using independently recovered OriginalMembers and
 * OriginalResources, registered CurrentMetadata, and exactly GetSize() readable
 * current bytes. Original member metadata MUST come from original evidence, not
 * a snapshot of the current metadata. This function cannot authenticate provenance.
 *
 * Full paths and semantic type/shape/precision must match; offsets may move.
 * Missing or changed leaves require complete typed values. There are no aliases,
 * inferred values or implicit semantic zeroes. Only ABI padding is zeroed.
 * Current-only members are allowed. All overrides must be used exactly once.
 * OriginalResources must preserve recovered tree traversal/array order; current
 * resources must match the current Engine's ascending-offset layout sequence.
 * Supported ABI: 32-bit numeric scalars/vectors/matrices, numeric array strides
 * rounded up to 16 bytes, inline structs/struct arrays, and 8-byte texture/SRV/UAV/sampler slots.
 * Unsupported types, malformed layouts, overlap and bounds violations fail.
 * Resource slots, including explicit nulls, are copied exactly from supplied
 * contents or complete overrides. Pointers are never dereferenced, authenticated
 * or interpreted as proof of resource validity or producer-value equivalence;
 * the binding caller decides whether a null is allowed for a particular slot.
 *
 * Output owns bytes ONLY. Resource handles remain borrowed: caller must retain
 * valid resource references through eventual RHI creation and use. Input metadata
 * and contents must remain stable during the call. No RHI buffer is created and
 * FMeshMaterialShader binding/provider compatibility is not established. Existing
 * SameUniform validation is unaffected. OutBytes is unchanged on failure.
 */
bool BuildLegacyOriginalUniformData(const FLegacyLocalUniformContract& Contract,
    const void* CurrentContents, uint32 CurrentContentsSize,
    const TArray<FLegacyOriginalUniformMemberValue>& Values,
    TArray<uint8>& OutBytes, FString& Error);
#endif
