#include "LegacyOriginalUniformBuffer.h"

#if PLATFORM_LINUX && !UE_SERVER
#include "DynamicRHI.h"
#include "RenderingThread.h"

namespace
{
// The inner shared pointer is moved here exclusively, before this anchor is
// published. Only the thread-safe anchor is shared across threads.
struct FLegacyOriginalUniformLayoutAnchor final : FRHIUniformBufferLayoutLifetime
{
    explicit FLegacyOriginalUniformLayoutAnchor(TSharedPtr<FLegacyOriginalUniformLayout>&& InLayout)
        : Layout(MoveTemp(InLayout)) {}
    TSharedPtr<FLegacyOriginalUniformLayout> Layout;
};

bool IsUniformCreationThread()
{
    return GRenderingThread ? IsInActualRenderingThread() : IsInGameThread();
}

bool Fail(FString& Error, const TCHAR* Why)
{
    Error = Why;
    return false;
}
}

FLegacyOriginalUniformBuffer::~FLegacyOriginalUniformBuffer() = default;

bool CreateLegacyOriginalUniformBuffer(const FLegacyLocalUniformContract& Contract,
    const void* CurrentContents, uint32 CurrentContentsSize,
    const TArray<FLegacyOriginalUniformMemberValue>& Values,
    EUniformBufferUsage Usage, EUniformBufferValidation Validation,
    TSharedPtr<FLegacyOriginalUniformBuffer>& Out, FString& Error)
{
    if (Out.IsValid()) return Fail(Error, TEXT("output uniform buffer owner must be empty"));
    if (!IsUniformCreationThread() || !GIsRHIInitialized || !GDynamicRHI)
        return Fail(Error, TEXT("uniform buffer creation requires the initialized RHI and rendering thread"));
    if (Usage != UniformBuffer_SingleDraw && Usage != UniformBuffer_SingleFrame && Usage != UniformBuffer_MultiFrame)
        return Fail(Error, TEXT("invalid uniform buffer usage"));
    if (Validation != EUniformBufferValidation::None && Validation != EUniformBufferValidation::ValidateResources)
        return Fail(Error, TEXT("invalid uniform resource validation policy"));

    TArray<uint8> Bytes;
    TSharedPtr<FLegacyOriginalUniformLayout> Layout;
    if (!BuildLegacyOriginalUniformData(Contract, CurrentContents, CurrentContentsSize, Values, Bytes, Error) ||
        !BuildLegacyOriginalUniformLayout(Contract, Layout, Error)) return false;
    const FRHIUniformBufferLayout& RHILayout = Layout->GetMetadata().GetLayout();
    if (Bytes.Num() <= 0 || uint32(Bytes.Num()) != Contract.OriginalSize ||
        RHILayout.ConstantBufferSize != uint32(Bytes.Num()))
        return Fail(Error, TEXT("reconstructed bytes do not match the original RHI layout"));
    for (const auto& Resource : RHILayout.Resources)
    {
        const uint32 Offset = Resource.MemberOffset;
        if (Offset > uint32(Bytes.Num()) || sizeof(void*) > uint32(Bytes.Num()) - Offset)
            return Fail(Error, TEXT("original resource slot is outside reconstructed bytes"));
        if (Validation == EUniformBufferValidation::ValidateResources)
        {
            void* Pointer = nullptr;
            FMemory::Memcpy(&Pointer, Bytes.GetData() + Offset, sizeof(Pointer));
            const bool bAllowedNullSRV = GMaxRHIFeatureLevel <= ERHIFeatureLevel::ES3_1 && Resource.MemberType == UBMT_SRV;
            if (!Pointer && !bAllowedNullSRV)
                return Fail(Error, TEXT("null original resource rejected by RHI validation policy"));
        }
    }

    TSharedPtr<FLegacyOriginalUniformBuffer> Result = MakeShareable(new FLegacyOriginalUniformBuffer());
    TSharedRef<FLegacyOriginalUniformLayoutAnchor, ESPMode::ThreadSafe> Anchor =
        MakeShared<FLegacyOriginalUniformLayoutAnchor, ESPMode::ThreadSafe>(MoveTemp(Layout));
    Result->Buffer = RHICreateUniformBuffer(Bytes.GetData(), RHILayout, Usage, Validation);
    if (!Result->Buffer.IsValid()) return Fail(Error, TEXT("RHI failed to create original uniform buffer"));
    // The buffer has not been published and therefore cannot be used on another
    // thread until the layout owner is installed.
    Result->Buffer->SetLayoutLifetimeOwner(Anchor);
    Out = MoveTemp(Result);
    Error.Reset();
    return true;
}
#endif
