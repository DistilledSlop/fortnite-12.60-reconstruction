#pragma once

#if PLATFORM_LINUX && !UE_SERVER
#include "LegacyOriginalUniformData.h"
#include "LegacyOriginalUniformLayout.h"
#include "RHIResources.h"

/** Owns an RHI reference to a reconstructed uniform buffer. The RHI buffer
 * itself retains its original metadata through actual RHI destruction, even
 * when other RHI references or deferred deletion outlive this owner. GetBuffer
 * is borrowed; a caller keeping it beyond this owner must take an RHI reference.
 * Neither this object nor its layout proves shader binding compatibility.
 */
class FLegacyOriginalUniformBuffer
{
public:
    ~FLegacyOriginalUniformBuffer();
    FRHIUniformBuffer* GetBuffer() const { return Buffer.GetReference(); }

private:
    friend bool CreateLegacyOriginalUniformBuffer(const FLegacyLocalUniformContract&,
        const void*, uint32, const TArray<FLegacyOriginalUniformMemberValue>&,
        EUniformBufferUsage, EUniformBufferValidation,
        TSharedPtr<FLegacyOriginalUniformBuffer>&, FString&);
    FLegacyOriginalUniformBuffer() = default;
    FLegacyOriginalUniformBuffer(const FLegacyOriginalUniformBuffer&) = delete;
    FLegacyOriginalUniformBuffer& operator=(const FLegacyOriginalUniformBuffer&) = delete;
    FUniformBufferRHIRef Buffer;
};

/** Reconstruct the old ABI bytes and metadata, then create the RHI buffer on
 * the rendering thread (or game thread with single-threaded rendering). Out
 * must be empty and remains unchanged on failure. Contents and metadata must be
 * stable throughout this call; all non-null resource pointers must be valid
 * RHI resources retained by the caller through creation. Validation is the
 * caller's explicit RHI null-resource policy; None does not establish semantic
 * equivalence of null values. Vulkan retains resources in its resource table.
 * Usage and Validation must be valid enum values. No binding is performed.
 */
bool CreateLegacyOriginalUniformBuffer(const FLegacyLocalUniformContract& Contract,
    const void* CurrentContents, uint32 CurrentContentsSize,
    const TArray<FLegacyOriginalUniformMemberValue>& Values,
    EUniformBufferUsage Usage, EUniformBufferValidation Validation,
    TSharedPtr<FLegacyOriginalUniformBuffer>& Out, FString& Error);
#endif
