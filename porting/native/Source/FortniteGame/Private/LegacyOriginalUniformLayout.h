#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyLocalVFAdapter.h"

/** Owns the original ABI metadata and all strings referenced by it. Keep this
 * owner alive until every RHI buffer made from its layout is no longer in use.
 * It does not own resource handles, create RHI objects, or establish binding
 * compatibility. The metadata is data-driven and is not globally registered.
 */
class FLegacyOriginalUniformLayout
{
public:
    ~FLegacyOriginalUniformLayout();
    const FShaderParametersMetadata& GetMetadata() const { return *Metadata; }

private:
    friend bool BuildLegacyOriginalUniformLayout(const FLegacyLocalUniformContract&,
        TSharedPtr<FLegacyOriginalUniformLayout>&, FString&);
    FLegacyOriginalUniformLayout() = default;
    FLegacyOriginalUniformLayout(const FLegacyOriginalUniformLayout&) = delete;
    FLegacyOriginalUniformLayout& operator=(const FLegacyOriginalUniformLayout&) = delete;
    const TCHAR* Retain(const FString& Value);
    const FShaderParametersMetadata* BuildChild(const FLegacyLocalMemberContract& Member);
    TArray<TUniquePtr<FString>> Strings;
    TArray<TUniquePtr<FShaderParametersMetadata>> Children;
    TUniquePtr<FShaderParametersMetadata> Metadata;
};

/** Construct independent metadata from original evidence; CurrentMetadata and
 * the recovered legacy resource hash are not used as layout proofs. Original
 * resource entries must be in tree traversal order. Requires the UE data-driven
 * inline-structure patch (porting/patches/ue425-legacy-datadriven-uniform-nesting.patch);
 * resource arrays remain unsupported and are rejected. Out is unchanged on failure; Error explains why.
 */
bool BuildLegacyOriginalUniformLayout(const FLegacyLocalUniformContract& Contract,
    TSharedPtr<FLegacyOriginalUniformLayout>& Out, FString& Error);
#endif
