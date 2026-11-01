#include "CoreMinimal.h"

#if PLATFORM_LINUX && !UE_SERVER
#include "DynamicRHI.h"
#include "Misc/Crc.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Materials/Material.h"
#include "Engine/Texture.h"
#include "TextureResource.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstance.h"
#include "RenderingThread.h"
#include "SceneView.h"
#include "RenderUtils.h"
#include "UnrealClient.h"
#include "ShowFlags.h"

RENDERER_API void UpdateNoiseTextureParameters(FViewUniformShaderParameters& Parameters);
#include "Rendering/RenderingCommon.h"
#include "PipelineStateCache.h"
#include "RHIStaticStates.h"
#include "Misc/FileHelper.h"
#include "Misc/SecureHash.h"
#include "LegacySlateMaterial.h"
#include "LegacyMaterialAsset.h"
#include "LegacyOriginalMaterialReader.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/MemoryWriter.h"
#include "VulkanCommon.h"
#include "vulkan.h"
#include "VulkanShaderResources.h"

// A descriptor's offsets are word offsets in the *renamed* module. The old
// offsets in the reflection receipt are invalid after changing OpEntryPoint.
struct FLegacySpirvDescriptor
{
    uint32 SetOffset = UINT32_MAX;
    uint32 BindingOffset = UINT32_MAX;
    uint32 Set = UINT32_MAX;
    uint32 Binding = UINT32_MAX;
};

static bool RewriteLegacySpirv(const FString& Path, const FString& ExpectedSha1,
    uint32 ExpectedModel, TArray<uint32>& Words, uint32& Crc,
    TMap<uint32, FLegacySpirvDescriptor>& Descriptors, FString& Error)
{
    TArray<uint8> Bytes;
    if (!FFileHelper::LoadFileToArray(Bytes, *Path) || Bytes.Num() < 20 || Bytes.Num() % 4 || Bytes.Num() > 4 * 1024 * 1024)
    { Error = TEXT("invalid SPIR-V file"); return false; }
    FSHAHash Hash;
    FSHA1::HashBuffer(Bytes.GetData(), Bytes.Num(), Hash.Hash);
    if (ExpectedSha1.Len() != 40 || !Hash.ToString().Equals(ExpectedSha1, ESearchCase::IgnoreCase))
    { Error = TEXT("SPIR-V digest mismatch"); return false; }
    Words.SetNumUninitialized(Bytes.Num() / 4);
    FMemory::Memcpy(Words.GetData(), Bytes.GetData(), Bytes.Num());
    if (Words[0] != 0x07230203 || Words[3] == 0 || Words[3] > 1000000)
    { Error = TEXT("invalid SPIR-V header"); return false; }
    int32 Entry = INDEX_NONE, OldNameWords = 0;
    for (int32 I = 5; I < Words.Num(); )
    {
        const uint32 Count = Words[I] >> 16, Op = Words[I] & 0xffff;
        if (!Count || Count > uint32(Words.Num() - I))
        { Error = TEXT("invalid SPIR-V instruction"); return false; }
        if (Op == 15)
        {
            if (Entry != INDEX_NONE || Count < 4 || Words[I + 1] != ExpectedModel)
            { Error = TEXT("unexpected SPIR-V entry point"); return false; }
            const ANSICHAR* Name = reinterpret_cast<const ANSICHAR*>(&Words[I + 3]);
            const int32 MaxBytes = (Count - 3) * 4;
            int32 Len = 0;
            while (Len < MaxBytes && Name[Len]) ++Len;
            if (Len != 4 || FCStringAnsi::Strncmp(Name, "main", 4) != 0)
            { Error = TEXT("unexpected SPIR-V entry point name"); return false; }
            Entry = I;
            OldNameWords = (Len + 4) / 4;
        }
        I += Count;
    }
    if (Entry == INDEX_NONE) { Error = TEXT("missing SPIR-V entry point"); return false; }
    Crc = FCrc::MemCrc32(Bytes.GetData(), Bytes.Num());
    constexpr int32 NewNameWords = 6;
    const uint32 NewBytes = (Words.Num() + NewNameWords - OldNameWords) * 4;
    ANSICHAR NewName[24] = {};
    FCStringAnsi::Snprintf(NewName, UE_ARRAY_COUNT(NewName), "main_%08x_%08x", NewBytes, Crc);
    Words.RemoveAt(Entry + 3, OldNameWords);
    Words.InsertZeroed(Entry + 3, NewNameWords);
    FMemory::Memcpy(&Words[Entry + 3], NewName, sizeof(NewName));
    Words[Entry] += (NewNameWords - OldNameWords) << 16;
    Descriptors.Reset();
    for (int32 I = 5; I < Words.Num(); )
    {
        const uint32 Count = Words[I] >> 16, Op = Words[I] & 0xffff;
        if (!Count || Count > uint32(Words.Num() - I))
        { Error = TEXT("invalid renamed SPIR-V instruction"); return false; }
        // OpDecorate: DescriptorSet = 34, Binding = 33.
        if (Op == 71 && Count >= 4 && (Words[I + 2] == 33 || Words[I + 2] == 34))
        {
            const uint32 Id = Words[I + 1];
            if (!Id || Id >= Words[3] || Count != 4)
            { Error = TEXT("invalid descriptor decoration"); return false; }
            FLegacySpirvDescriptor& Descriptor = Descriptors.FindOrAdd(Id);
            uint32& Offset = Words[I + 2] == 33 ? Descriptor.BindingOffset : Descriptor.SetOffset;
            uint32& Value = Words[I + 2] == 33 ? Descriptor.Binding : Descriptor.Set;
            if (Offset != UINT32_MAX) { Error = TEXT("duplicate descriptor decoration"); return false; }
            Offset = I + 3; Value = Words[I + 3];
        }
        I += Count;
    }
    for (const auto& Pair : Descriptors)
        if (Pair.Value.SetOffset == UINT32_MAX || Pair.Value.BindingOffset == UINT32_MAX)
        { Error = TEXT("incomplete descriptor decoration"); return false; }
    return true;
}

static bool PrepareLegacyOutput(const TSharedPtr<FJsonObject>& Shader, const FRHIUniformBufferLayout& Layout,
    FLegacySlateShaderInput& Input, FString& Error, bool bWriteArtifacts = true, bool bHasView = false)
{
    const TSharedPtr<FJsonObject> Record = Shader->GetObjectField(TEXT("record"));
    const TSharedPtr<FJsonObject> Reflection = Shader->GetObjectField(TEXT("reflection"));
    if (!Record || !Reflection) { Error = TEXT("missing shader record"); return false; }
    const TSharedPtr<FJsonObject> Type = Record->GetObjectField(TEXT("type"));
    const TSharedPtr<FJsonObject> Resource = Record->GetObjectField(TEXT("resource"));
    if (!Type || !Reflection || !Resource) { Error = TEXT("missing shader metadata"); return false; }
    Input.TypeName = Type->GetStringField(TEXT("name"));
    const int32 Frequency = int32(Record->GetNumberField(TEXT("frequency")));
    if (Frequency != 0 && Frequency != 3) { Error = TEXT("invalid shader frequency"); return false; }
    const bool bVertex = Frequency == 0;
    Input.PermutationId = int32(Record->GetNumberField(TEXT("permutation_id")));
    TArray<uint32> Spirv;
    TMap<uint32, FLegacySpirvDescriptor> Decorations;
    uint32 Crc = 0;
    if (!RewriteLegacySpirv(Reflection->GetStringField(TEXT("output")), Shader->GetStringField(TEXT("spirv_sha1")),
        bVertex ? 0 : 4, Spirv, Crc, Decorations, Error)) return false;

    FVulkanShaderHeader Header(FVulkanShaderHeader::EZero);
    Header.bHasRealUBs = true;
    Header.SpirvCRC = Crc;
    const auto& DescriptorValues = Reflection->GetArrayField(TEXT("descriptors"));
    if (DescriptorValues.Num() != Decorations.Num()) { Error = TEXT("descriptor count mismatch"); return false; }
    bool bPacked = false, bMaterial = false, bView = false, bSampler = false, bTexture = false;
    int32 MaterialIndex = INDEX_NONE, SamplerCount = 0;
    TArray<FVulkanShaderHeader::FUBResourceInfo> MaterialResources;
    for (const TSharedPtr<FJsonValue>& Value : DescriptorValues)
    {
        const TSharedPtr<FJsonObject> D = Value->AsObject();
        if (!D) { Error = TEXT("invalid descriptor"); return false; }
        const uint32 Id = uint32(D->GetNumberField(TEXT("variable_id")));
        const FLegacySpirvDescriptor* S = Decorations.Find(Id);

        if (!S || S->Set != uint32(D->GetNumberField(TEXT("set"))) || S->Binding != uint32(D->GetNumberField(TEXT("binding"))) || S->Set != 0 || S->Binding > UINT16_MAX )
        { Error = TEXT("descriptor receipt mismatch"); return false; }
        const auto Register = D->GetObjectField(TEXT("d3d_register"));
        if (!Register || int32(Register->GetNumberField(TEXT("space"))) != 0)
        { Error = TEXT("D3D register receipt mismatch"); return false; }
        const FString Kind = Register->GetStringField(TEXT("kind"));
        const int32 RegisterIndex = int32(Register->GetNumberField(TEXT("index")));
        const int32 DescriptorType = int32(D->GetNumberField(TEXT("vulkan_descriptor_type")));
        if (Kind == TEXT("cbv") && D->GetArrayField(TEXT("resource_table_entries")).Num()) { Error=TEXT("unexpected CBV resource entry"); return false; }
        if (Kind == TEXT("cbv") && RegisterIndex == 0 && !bPacked && DescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        {
            const int32 Span = int32(D->GetNumberField(TEXT("block_span_bytes")));
            if (Span != (bVertex ? 80 : 64)) { Error = TEXT("unexpected packed buffer span"); return false; }
            auto& P = Header.PackedUBs.AddDefaulted_GetRef();
            P.SizeInBytes = Span; P.OriginalBindingIndex = S->Binding;
            P.PackedTypeIndex = CrossCompiler::EPackedTypeIndex::HighP;
            P.SPIRVDescriptorSetOffset = S->SetOffset; P.SPIRVBindingIndexOffset = S->BindingOffset;
            bPacked = true;
        }
        else if (Kind == TEXT("cbv") && RegisterIndex == 1 && bHasView && !bVertex && !bView && DescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        {
            const FShaderParametersMetadata* ViewMetadata=FViewUniformShaderParameters::FTypeInfo::GetStructMetadata();
            if (STRUCT_OFFSET(FViewUniformShaderParameters, GameTime)!=2264 ||
                STRUCT_OFFSET(FViewUniformShaderParameters, RealTime)!=2268 ||
                int32(D->GetNumberField(TEXT("block_span_bytes")))!=2272 || !ViewMetadata ||
                !ViewMetadata->GetLayout().GetHash())
            { Error=TEXT("View layout does not match original GameTime contract"); return false; }
            const uint16 Index=Header.UniformBuffers.Num();
            auto& U=Header.UniformBuffers.AddDefaulted_GetRef();
            U.LayoutHash=ViewMetadata->GetLayout().GetHash(); U.ConstantDataOriginalBindingIndex=S->Binding; U.bOnlyHasResources=0;
            Header.UniformBufferSpirvInfos.Emplace(S->SetOffset,S->BindingOffset);
            Input.Output.ParameterMap.AddParameterAllocation(TEXT("View"),Index,FVulkanShaderHeader::UniformBuffer,1,EShaderParameterType::UniformBuffer);
            bView=true;
        }
        else if (Kind == TEXT("cbv") && RegisterIndex == (bHasView ? 2 : 1) && !bVertex && !bMaterial && DescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        {
            if (!Layout.Resources.Num() || int32(D->GetNumberField(TEXT("block_span_bytes"))) > int32(Layout.Resources[0].MemberOffset) || (Layout.Resources.Num()==2 && int32(D->GetNumberField(TEXT("block_span_bytes"))) != int32(Layout.Resources[0].MemberOffset)) || Layout.GetHash() == 0)
            { Error = TEXT("unexpected Material descriptor"); return false; }
            const uint16 Index=Header.UniformBuffers.Num();
            MaterialIndex=Index;
            auto& U = Header.UniformBuffers.AddDefaulted_GetRef();
            U.LayoutHash = Layout.GetHash(); U.ConstantDataOriginalBindingIndex = S->Binding; U.bOnlyHasResources = 0;
            Header.UniformBufferSpirvInfos.Emplace(S->SetOffset, S->BindingOffset);
            Input.Output.ParameterMap.AddParameterAllocation(TEXT("Material"), Index, FVulkanShaderHeader::UniformBuffer, 1, EShaderParameterType::UniformBuffer);
            bMaterial = true;
        }
        else if ((Kind == TEXT("sampler") && DescriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) ||
                 (Kind == TEXT("srv") && DescriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE))
        {
            if (bVertex) { Error = TEXT("unexpected vertex descriptor"); return false; }
            const bool bIsSampler = Kind == TEXT("sampler");
            if (bIsSampler) ++SamplerCount;
            const auto& Entries = D->GetArrayField(TEXT("resource_table_entries"));
            if (Entries.Num() > 1) { Error=TEXT("ambiguous resource table binding"); return false; }
            const uint16 Index = Header.Globals.Num();
            auto& G = Header.Globals.AddDefaulted_GetRef();
            G.OriginalBindingIndex = S->Binding; G.CombinedSamplerStateAliasIndex = UINT16_MAX;
            G.TypeIndex = Header.GlobalDescriptorTypes.Add(static_cast<VkDescriptorType>(DescriptorType));
            Header.GlobalSpirvInfos.Emplace(S->SetOffset, S->BindingOffset);
            if (Entries.Num())
            {
                const auto Entry=Entries[0]->AsObject();
                const int32 ResourceIndex=int32(Entry->GetNumberField(TEXT("resource")));
                const int32 SourceUB=int32(Entry->GetNumberField(TEXT("uniform_buffer")));
                if (SourceUB != (bHasView ? 2 : 1) || int32(Entry->GetNumberField(TEXT("bind"))) != RegisterIndex ||
                    ResourceIndex < 0 || ResourceIndex >= Layout.Resources.Num() ||
                    Entry->GetStringField(TEXT("table")) != (bIsSampler ? TEXT("sampler") : TEXT("texture")) ||
                    Layout.Resources[ResourceIndex].MemberType != (bIsSampler ? UBMT_SAMPLER : UBMT_TEXTURE))
                { Error=TEXT("invalid Material resource table entry"); return false; }
                auto& R=MaterialResources.AddDefaulted_GetRef();
                R.SourceUBResourceIndex=ResourceIndex; R.OriginalBindingIndex=RegisterIndex;
                R.GlobalIndex=Index; R.UBBaseType=bIsSampler ? UBMT_SAMPLER : UBMT_TEXTURE;
            }
            else
            {
                if ((bIsSampler && bSampler) || (!bIsSampler && bTexture)) { Error=TEXT("duplicate element texture"); return false; }
                Input.Output.ParameterMap.AddParameterAllocation(bIsSampler ? TEXT("ElementTextureSampler") : TEXT("ElementTexture"),
                    FVulkanShaderHeader::Global, Index, 1, bIsSampler ? EShaderParameterType::Sampler : EShaderParameterType::SRV);
                if (bIsSampler) bSampler = true; else bTexture = true;
            }
        }
        else { Error = TEXT("unsupported descriptor"); return false; }
    }
    if (!bVertex && Layout.Resources.Num()==4 && MaterialResources.Num()!=2) { Error=TEXT("incomplete Material resource table"); return false; }
    if (MaterialResources.Num())
    {
        if (MaterialIndex == INDEX_NONE) { Error=TEXT("resource table without Material buffer"); return false; }
        Header.UniformBuffers[MaterialIndex].ResourceEntries=MoveTemp(MaterialResources);
    }
    if (!bPacked || bMaterial == bVertex || bView != (bHasView && !bVertex) || bSampler != bTexture)
    { Error = TEXT("missing shader descriptor"); return false; }
    const TCHAR* VsNames[] = {TEXT("ViewProjection"), TEXT("SwitchVerticalAxisMultiplier")};
    const TCHAR* PsNames[] = {TEXT("GammaAndAlphaValues"), TEXT("ShaderParams"), TEXT("DrawFlags")};
    const auto& Parameters = Record->GetArrayField(TEXT("scalar_parameters"));
    if (Parameters.Num() != (bVertex ? 2 : 3)) { Error = TEXT("scalar parameter count mismatch"); return false; }
    for (int32 I = 0; I < Parameters.Num(); ++I)
    {
        const auto P = Parameters[I]->AsObject();
        const int32 Offset = int32(P->GetNumberField(TEXT("base_index")));
        const int32 Size = int32(P->GetNumberField(TEXT("byte_count")));
        if (int32(P->GetNumberField(TEXT("buffer_index"))) != 0 || Offset < 0 || Size < 0 || Offset + Size > (bVertex ? 80 : 64) || Size % 4)
        { Error = TEXT("invalid loose parameter"); return false; }
        if (Size)
        {
            const TCHAR* Name = bVertex ? VsNames[I] : PsNames[I];
            Input.Output.ParameterMap.AddParameterAllocation(Name, 0, Offset, Size, EShaderParameterType::LooseData);
            auto& G = Header.PackedGlobals.AddDefaulted_GetRef();
            G.ConstantDataSizeInFloats = Size / 4; G.PackedTypeIndex = CrossCompiler::EPackedTypeIndex::HighP; G.PackedUBIndex = 0;
        }
    }
    for (const auto& Value : Reflection->GetArrayField(TEXT("interfaces")))
    {
        const auto Interface = Value->AsObject();
        double Location;
        if (Interface->GetStringField(TEXT("direction")) == (bVertex ? TEXT("input") : TEXT("output")) && Interface->TryGetNumberField(TEXT("location"), Location))
        {
            if (Location < 0 || Location >= 15 || Location != int32(Location)) { Error = TEXT("invalid interface location"); return false; }
            Header.InOutMask |= 1u << int32(Location);
        }
    }
    if (!Header.InOutMask) { Error = TEXT("missing shader interface mask"); return false; }
    Input.Output.Target = FShaderTarget(bVertex ? SF_Vertex : SF_Pixel, SP_VULKAN_SM5);
    Input.Output.NumInstructions = uint32(Resource->GetNumberField(TEXT("instruction_count")));
    Input.Output.NumTextureSamplers = SamplerCount;
    FMemoryWriter Writer(Input.Output.ShaderCode.GetWriteAccess(), true);
    Writer << Header << Spirv;
    if (Writer.IsError()) { Error = TEXT("failed to serialize Vulkan shader"); return false; }
    Input.Output.bSucceeded = true;
    Input.Output.GenerateOutputHash();
    FString ArtifactDirectory;
    if (bWriteArtifacts && FParse::Value(FCommandLine::Get(), TEXT("FortniteLegacyMaterialArtifactDir="), ArtifactDirectory))
    {
        if (!FPaths::DirectoryExists(ArtifactDirectory) || Input.TypeName.IsEmpty() ||
            FPaths::GetCleanFilename(Input.TypeName) != Input.TypeName)
        { Error = TEXT("invalid artifact directory or name"); return false; }
        const FString SpirvPath = FPaths::Combine(ArtifactDirectory, Input.TypeName + TEXT(".spv"));
        const FString CodePath = FPaths::Combine(ArtifactDirectory, Input.TypeName + TEXT(".ue-vulkan"));
        if (FPaths::FileExists(SpirvPath) || FPaths::FileExists(CodePath))
        { Error = TEXT("artifact already exists"); return false; }
        TArray<uint8> SpirvBytes;
        SpirvBytes.Append(reinterpret_cast<const uint8*>(Spirv.GetData()), Spirv.Num() * sizeof(uint32));
        if (!FFileHelper::SaveArrayToFile(SpirvBytes, *SpirvPath) ||
            !FFileHelper::SaveArrayToFile(Input.Output.ShaderCode.GetWriteAccess(), *CodePath))
        { Error = TEXT("cannot write shader artifacts"); return false; }
    }
    return true;
}

struct FLegacyOverride
{
    FHashedMaterialParameterInfo Parameter;
    bool bVector = false;
    FLinearColor Value;
};
struct FLegacyUniformCase
{
    FString Name;
    TArray<FLegacyOverride> Overrides;
    TArray<float> Expected;
};
class FLegacyProbeProxy : public FMaterialRenderProxy
{
public:
    const FMaterialRenderProxy* Base = nullptr;
    TArray<FLegacyOverride> Overrides;
    bool bInherit = false;
    const UTexture* TextureOverride = nullptr;
    virtual const FMaterial& GetMaterialWithFallback(ERHIFeatureLevel::Type Feature, const FMaterialRenderProxy*& Fallback) const override
    { return Base->GetMaterialWithFallback(Feature, Fallback); }
    virtual bool GetVectorValue(const FHashedMaterialParameterInfo& Info, FLinearColor* Value, const FMaterialRenderContext& Context) const override
    { for (const auto& O : Overrides) if (O.bVector && O.Parameter == Info) { *Value = O.Value; return true; } return bInherit && Base->GetVectorValue(Info, Value, Context); }
    virtual bool GetScalarValue(const FHashedMaterialParameterInfo& Info, float* Value, const FMaterialRenderContext& Context) const override
    { for (const auto& O : Overrides) if (!O.bVector && O.Parameter == Info) { *Value = O.Value.R; return true; } return bInherit && Base->GetScalarValue(Info, Value, Context); }
    virtual bool GetTextureValue(const FHashedMaterialParameterInfo& Info, const UTexture** Value, const FMaterialRenderContext& Context) const override { if (TextureOverride && Info==FHashedMaterialParameterInfo(TEXT("Icon"))) { *Value=TextureOverride; return true; } return bInherit && Base->GetTextureValue(Info, Value, Context); }
    virtual bool GetTextureValue(const FHashedMaterialParameterInfo&, const URuntimeVirtualTexture**, const FMaterialRenderContext&) const override { return false; }
};

static bool CheckLegacyUniforms(const FUniformExpressionSet& Uniforms, const TSharedPtr<FJsonObject>& Root, const TCHAR* Stage, FString& Error, UMaterialInterface* Owner = nullptr, const TCHAR* CasesField = TEXT("uniform_cases"))
{
    TArray<FLegacyUniformCase> Cases;
    const auto& Values = Root->GetArrayField(CasesField);
    if (Values.Num() < 1 || Values.Num() > 64) { Error = TEXT("invalid uniform cases"); return false; }
    const int32 NumericSize = int32(Root->GetObjectField(TEXT("expected_layout"))->GetArrayField(TEXT("resource_offsets"))[0]->AsNumber());
    const int32 MeaningfulValues = Root->GetArrayField(TEXT("vector_roots")).Num() * 4 + Root->GetArrayField(TEXT("scalar_roots")).Num();
    if (NumericSize <= 0 || NumericSize > 4096 || MeaningfulValues > NumericSize / 4) { Error = TEXT("invalid numeric layout"); return false; }
    for (const auto& Value : Values)
    {
        const auto Case = Value->AsObject();
        auto& C = Cases.AddDefaulted_GetRef();
        C.Name = Case->GetStringField(TEXT("name"));
        const auto& Expected = Case->GetArrayField(TEXT("numeric_values"));
        if (Expected.Num() != NumericSize / 4) { Error = TEXT("invalid numeric reference"); return false; }
        for (const auto& V : Expected) C.Expected.Add(float(V->AsNumber()));
        for (const auto& OverrideValue : Case->GetArrayField(TEXT("overrides")))
        {
            const auto O = OverrideValue->AsObject();
            const auto P = O->GetObjectField(TEXT("parameter"));
            const auto N = P->GetObjectField(TEXT("name"));
            const FName Name(*N->GetStringField(TEXT("name")), int32(N->GetNumberField(TEXT("number"))));
            auto& Out = C.Overrides.AddDefaulted_GetRef();
            Out.Parameter = FHashedMaterialParameterInfo(Name, static_cast<EMaterialParameterAssociation>(int32(P->GetNumberField(TEXT("association")))), int32(P->GetNumberField(TEXT("index"))));
            Out.bVector = O->GetStringField(TEXT("type")) == TEXT("vector");
            if (Out.bVector)
            {
                const auto& V = O->GetArrayField(TEXT("value"));
                if (V.Num() != 4) { Error = TEXT("invalid vector override"); return false; }
                Out.Value = FLinearColor(V[0]->AsNumber(), V[1]->AsNumber(), V[2]->AsNumber(), V[3]->AsNumber());
            }
            else Out.Value.R = O->GetNumberField(TEXT("value"));
        }
    }
    UMaterialInterface* DefaultMaterial = Owner ? Owner : UMaterial::GetDefaultMaterial(MD_Surface);
    if (!DefaultMaterial) { Error = TEXT("default material unavailable"); return false; }
    const FMaterialRenderProxy* Base = DefaultMaterial->GetRenderProxy();
    bool bPassed = true;
    FString Failure;
    ENQUEUE_RENDER_COMMAND(CheckLegacyUniformExpressions)([&Uniforms, &Cases, Base, Owner, MeaningfulValues, &bPassed, &Failure](FRHICommandListImmediate&)
    {
        FLegacyProbeProxy Proxy;
        Proxy.Base = Base; Proxy.bInherit = Owner != nullptr;
        const FMaterialRenderProxy* Fallback = nullptr;
        const FMaterial& Material = Base->GetMaterialWithFallback(ERHIFeatureLevel::SM5, Fallback);
        FMaterialRenderContext Context(&Proxy, Material, nullptr);
        FUniformExpressionCache Cache;
        for (const auto& C : Cases)
        {
            Proxy.Overrides = C.Overrides;
            alignas(16) uint8 Buffer[4112] = {};
            Uniforms.FillUniformBuffer(Context, Cache, Buffer, sizeof(Buffer));
            const float* Actual = reinterpret_cast<const float*>(Buffer);
            for (int32 I = 0; I < MeaningfulValues; ++I)
            {
                const float Expected = C.Expected[I];
                if (!FMath::IsFinite(Actual[I]) || FMath::Abs(Actual[I] - Expected) > 1.e-5f + 1.e-5f * FMath::Abs(Expected))
                { bPassed = false; Failure = FString::Printf(TEXT("uniform case %s slot %d: actual %g expected %g"), *C.Name, I, Actual[I], Expected); return; }
            }
        }
    });
    FlushRenderingCommands();
    if (!bPassed) { Error = Failure; return false; }
    UE_LOG(LogTemp, Display, TEXT("Legacy material probe: %d %s live uniform cases passed"), Cases.Num(), Stage);
    return true;
}

struct FLegacyDrawBindings
{
    uint16 MatrixBuffer = 0, MatrixOffset = 0, GammaBuffer = 0, GammaOffset = 0;
    uint16 ParamsBuffer = 0, ParamsOffset = 0, MaterialBuffer = 0, ViewBuffer = 0;
};

static bool DrawLegacySlateQuad(const TRefCountPtr<FMaterialShaderMap>& Map, const FLegacyDrawBindings& Bindings, FString& Error, UMaterialInterface* Owner = nullptr, const FMaterial* ExpectedOwnerResource = nullptr, const TCHAR* OverrideName = TEXT("RBGA1"), const TCHAR* ArtifactPrefix = TEXT("LockerQuad"), const TSharedPtr<FJsonObject>& ViewFixture = nullptr, bool bColorDraw = false, const UTexture* IconOverride = nullptr)
{
    FShaderType* VertexType = FindShaderTypeByName(FHashedName(TEXT("TSlateMaterialShaderVSfalse")));
    FShaderType* PixelType = FindShaderTypeByName(FHashedName(TEXT("TSlateMaterialShaderPSDefault")));
    UMaterialInterface* DefaultMaterial = Owner ? Owner : UMaterial::GetDefaultMaterial(MD_Surface);
    if (!VertexType || !PixelType || !DefaultMaterial) { Error = TEXT("draw shader or material unavailable"); return false; }
    const FMaterialRenderProxy* Base = DefaultMaterial->GetRenderProxy();
    struct FViewCase { float Game, Real; };
    TArray<FViewCase> ViewCases;
    if (ViewFixture)
    {
        const auto Contract=ViewFixture->GetObjectField(TEXT("view_contract"));
        if (!Contract || Contract->GetStringField(TEXT("buffer_name"))!=TEXT("View") ||
            Contract->GetStringField(TEXT("field_name"))!=TEXT("GameTime") ||
            int32(Contract->GetNumberField(TEXT("original_byte_offset")))!=2264 ||
            int32(Contract->GetNumberField(TEXT("expected_current_byte_offset")))!=2264 ||
            int32(Contract->GetNumberField(TEXT("shader_visible_bytes")))!=2272)
        { Error=TEXT("invalid View contract"); return false; }
        for (const auto& Value : ViewFixture->GetArrayField(TEXT("view_cases")))
        {
            const auto Case=Value->AsObject();
            const float Game=Case->GetNumberField(TEXT("game_time")), Real=Case->GetNumberField(TEXT("real_time"));
            if (Game!=float(Case->GetNumberField(TEXT("expected_game_time"))) || Real!=float(Case->GetNumberField(TEXT("expected_real_time"))))
            { Error=TEXT("invalid View time expectation"); return false; }
            ViewCases.Add({Game,Real});
        }
        if (ViewCases.Num()!=4) { Error=TEXT("invalid View case count"); return false; }
    }
    bool bPassed = true;
    FString Failure;
    TArray<FColor> Images[4];
    int32 Different = 0;
    ENQUEUE_RENDER_COMMAND(DrawOriginalLockerSlate)([Map, VertexType, PixelType, Base, Bindings, ExpectedOwnerResource, OverrideName, ViewCases, bColorDraw, IconOverride, &bPassed, &Failure, &Images, &Different](FRHICommandListImmediate& RHICmdList)
    {
        const TShaderRef<FShader> VS = Map->GetShader(VertexType, 0);
        const TShaderRef<FShader> PS = Map->GetShader(PixelType, 0);
        if (!VS.IsValid() || !PS.IsValid()) { bPassed = false; Failure = TEXT("missing draw shader"); return; }
        FVertexDeclarationElementList Elements;
        const uint32 Stride = sizeof(FSlateVertex);
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FSlateVertex, TexCoords), VET_Float4, 0, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FSlateVertex, MaterialTexCoords), VET_Float2, 1, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FSlateVertex, Position), VET_Float2, 2, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FSlateVertex, Color), VET_Color, 3, Stride));
        Elements.Add(FVertexElement(0, STRUCT_OFFSET(FSlateVertex, PixelSize), VET_UShort2, 4, Stride));
        const FVertexDeclarationRHIRef Declaration = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
        FSlateVertex Vertices[6];
        const FVector2D Positions[] = {{-1,-1}, {1,-1}, {-1,1}, {-1,1}, {1,-1}, {1,1}};
        const FVector2D UV[] = {{0,1}, {1,1}, {0,0}, {0,0}, {1,1}, {1,0}};
        for (int32 I = 0; I < 6; ++I)
        {
            Vertices[I].Position = Positions[I]; Vertices[I].MaterialTexCoords = UV[I];
            Vertices[I].TexCoords[0] = UV[I].X; Vertices[I].TexCoords[1] = UV[I].Y;
            Vertices[I].TexCoords[2] = 1.f; Vertices[I].TexCoords[3] = 1.f;
            Vertices[I].Color = FColor::White; Vertices[I].PixelSize[0] = 64; Vertices[I].PixelSize[1] = 64;
        }
        FRHIResourceCreateInfo VertexInfo;
        FVertexBufferRHIRef VertexBuffer = RHICreateVertexBuffer(sizeof(Vertices), BUF_Static, VertexInfo);
        void* VertexData = RHILockVertexBuffer(VertexBuffer, 0, sizeof(Vertices), RLM_WriteOnly);
        FMemory::Memcpy(VertexData, Vertices, sizeof(Vertices));
        RHIUnlockVertexBuffer(VertexBuffer);
        FRHIResourceCreateInfo TargetInfo(FClearValueBinding::Transparent);
        FTexture2DRHIRef Target = RHICreateTexture2D(64, 64, PF_B8G8R8A8, 1, 1, TexCreate_RenderTargetable | TexCreate_ShaderResource, TargetInfo);
        FLegacyProbeProxy Proxy; Proxy.Base = Base; Proxy.bInherit = ExpectedOwnerResource != nullptr;
        const FMaterialRenderProxy* Fallback = nullptr;
        const FMaterial& Material = Base->GetMaterialWithFallback(ERHIFeatureLevel::SM5, Fallback);
        if (ExpectedOwnerResource && &Material != ExpectedOwnerResource) { bPassed=false; Failure=TEXT("owner render proxy selected a fallback resource"); return; }
        FMaterialRenderContext Context(&Proxy, Material, nullptr);
        FUniformExpressionCache Cache;
        for (int32 Pass = 0; Pass < (ViewCases.Num() && !bColorDraw ? 4 : 2); ++Pass)
        {
            Proxy.Overrides.Reset();
            Proxy.TextureOverride=Pass==1 ? IconOverride : nullptr;
            if (IconOverride) {}
            else if (ViewCases.Num())
            {
                auto& O=Proxy.Overrides.AddDefaulted_GetRef();
                O.Parameter=FHashedMaterialParameterInfo(TEXT("s_UseShineSwipe"));
                O.bVector=false; O.Value.R=1.f;
                if (bColorDraw)
                {
                    auto& Color=Proxy.Overrides.AddDefaulted_GetRef();
                    Color.Parameter=FHashedMaterialParameterInfo(OverrideName); Color.bVector=true;
                    Color.Value=Pass==0 ? FLinearColor(1,0,0,1) : FLinearColor(0,1,0,1);
                }
            }
            else for (const TCHAR* Name : {OverrideName})
            {
                auto& O = Proxy.Overrides.AddDefaulted_GetRef();
                O.Parameter = FHashedMaterialParameterInfo(Name);
                O.bVector = true;
                O.Value = Pass == 0 ? FLinearColor(1, 0, 0, 1) : FLinearColor(0, 1, 0, 1);
            }
            alignas(16) uint8 Contents[4112] = {};
            const FUniformExpressionSet& Uniforms = Map->GetUniformExpressionSet();
            Uniforms.FillUniformBuffer(Context, Cache, Contents, sizeof(Contents));
            if (IconOverride && Pass==1)
            {
                void* Actual=nullptr; FMemory::Memcpy(&Actual,Contents+96,sizeof(void*));
                if (!IconOverride->Resource || !IconOverride->TextureReference.TextureReferenceRHI || !IconOverride->TextureReference.TextureReferenceRHI->GetReferencedTexture() ||
                    IconOverride->TextureReference.TextureReferenceRHI->GetReferencedTexture()==GWhiteTexture->TextureRHI.GetReference() || Actual!=IconOverride->TextureReference.TextureReferenceRHI.GetReference())
                { bPassed=false; Failure=TEXT("Icon override did not bind its texture"); return; }
            }
            FUniformBufferRHIRef MaterialUB = RHICreateUniformBuffer(Contents, Uniforms.GetUniformBufferLayout(), UniformBuffer_SingleDraw);
            RHICmdList.TransitionResource(EResourceTransitionAccess::EWritable, Target);
            FRHIRenderPassInfo PassInfo(Target, ERenderTargetActions::Clear_Store);
            RHICmdList.BeginRenderPass(PassInfo, TEXT("OriginalLockerSlate"));
            FGraphicsPipelineStateInitializer PSO;
            RHICmdList.ApplyCachedRenderTargets(PSO);
            PSO.BlendState = TStaticBlendState<>::GetRHI();
            PSO.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
            PSO.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
            PSO.PrimitiveType = PT_TriangleList;
            PSO.BoundShaderState.VertexDeclarationRHI = Declaration;
            PSO.BoundShaderState.VertexShaderRHI = VS.GetVertexShader();
            PSO.BoundShaderState.PixelShaderRHI = PS.GetPixelShader();
            SetGraphicsPipelineState(RHICmdList, PSO);
            RHICmdList.SetViewport(0, 0, 0, 64, 64, 1);
            const FMatrix Matrix = FMatrix::Identity;
            const FVector4 Gamma(1.f, 1.f / 2.2f, 0.f, 1.f);
            const FVector4 Params(0.f, 0.f, 0.f, 0.f);
            RHICmdList.SetShaderParameter(VS.GetVertexShader(), Bindings.MatrixBuffer, Bindings.MatrixOffset, sizeof(Matrix), &Matrix);
            RHICmdList.SetShaderParameter(PS.GetPixelShader(), Bindings.GammaBuffer, Bindings.GammaOffset, sizeof(Gamma), &Gamma);
            RHICmdList.SetShaderParameter(PS.GetPixelShader(), Bindings.ParamsBuffer, Bindings.ParamsOffset, sizeof(Params), &Params);
            RHICmdList.SetShaderUniformBuffer(PS.GetPixelShader(), Bindings.MaterialBuffer, MaterialUB);
            TUniformBufferRef<FViewUniformShaderParameters> ViewUB;
            if (ViewCases.Num())
            {
                class FProbeTarget final : public FRenderTarget
                {
                public:
                    explicit FProbeTarget(const FTexture2DRHIRef& Texture) { RenderTargetTextureRHI=Texture; }
                    virtual FIntPoint GetSizeXY() const override { return FIntPoint(64,64); }
                } ProbeTarget(Target);
                FEngineShowFlags Flags(ESFIM_Game);
                FSceneViewFamilyContext Family(FSceneViewFamily::ConstructionValues(&ProbeTarget,nullptr,Flags)
                    .SetWorldTimes(ViewCases[bColorDraw ? 1 : Pass].Game,0.f,ViewCases[bColorDraw ? 1 : Pass].Real).SetRealtimeUpdate(true));
                FSceneViewInitOptions Options;
                const FIntRect Rect(0,0,64,64);
                Options.ViewFamily=&Family; Options.SetViewRectangle(Rect); Options.ViewOrigin=FVector::ZeroVector;
                Options.ViewRotationMatrix=FMatrix::Identity; Options.ProjectionMatrix=FMatrix::Identity;
                Options.BackgroundColor=FLinearColor::Black; Options.OverlayColor=FLinearColor::White;
                FSceneView* View=new FSceneView(Options); Family.Views.Add(View);
                FViewUniformShaderParameters Parameters;
                Parameters.VTFeedbackBuffer=GEmptyVertexBufferWithUAV->UnorderedAccessViewRHI;
                View->SetupCommonViewUniformBufferParameters(Parameters,FIntPoint(64,64),1,Rect,View->ViewMatrices,FViewMatrices());
                Parameters.WorldViewOrigin=View->ViewMatrices.GetViewOrigin();
                Parameters.MobilePreviewMode=0.f;
                UpdateNoiseTextureParameters(Parameters);
                if (Parameters.GameTime!=ViewCases[bColorDraw ? 1 : Pass].Game || Parameters.RealTime!=ViewCases[bColorDraw ? 1 : Pass].Real)
                { bPassed=false; Failure=TEXT("scene View times differ from receipt"); return; }
                ViewUB=TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(Parameters,UniformBuffer_SingleFrame);
                RHICmdList.SetShaderUniformBuffer(PS.GetPixelShader(),Bindings.ViewBuffer,ViewUB);
            }
            RHICmdList.SetStreamSource(0, VertexBuffer, 0);
            RHICmdList.DrawPrimitive(0, 2, 1);
            RHICmdList.EndRenderPass();
            RHICmdList.TransitionResource(EResourceTransitionAccess::EReadable, Target);
            FReadSurfaceDataFlags Flags(RCM_UNorm); Flags.SetLinearToGamma(false);
            RHICmdList.ReadSurfaceData(Target, FIntRect(0, 0, 64, 64), Images[Pass], Flags);
        }
        if (Images[0].Num() != 4096 || Images[1].Num() != 4096)
        { bPassed = false; Failure = TEXT("invalid draw readback size"); return; }
        for (int32 I = 0; I < 4096; ++I) Different += Images[0][I] != Images[1][I] ? 1 : 0;
        if (!Different) { bPassed = false; Failure = TEXT("draws produced identical pixels"); }
        if (ViewCases.Num() && !bColorDraw && (Images[2].Num()!=4096 || Images[3].Num()!=4096 || Images[0]!=Images[2] || Images[1]!=Images[3] || Images[2]==Images[3]))
        { bPassed=false; Failure=TEXT("View time field isolation failed"); }
        UE_LOG(LogTemp, Display, TEXT("Legacy material probe: original Slate quad readback changed %d pixels"), Different);
    });
    FlushRenderingCommands();
    FString ArtifactDirectory;
    if (Images[0].Num() == 4096 && Images[1].Num() == 4096 && FParse::Value(FCommandLine::Get(), TEXT("FortniteLegacyMaterialArtifactDir="), ArtifactDirectory))
    {
        FSHAHash Hashes[4];
        const int32 Count=ViewCases.Num() && !bColorDraw ? 4 : 2;
        FString CaseJson;
        for (int32 I = 0; I < Count; ++I)
        {
            const FString Suffix=ViewCases.Num() && !bColorDraw ? FString::Printf(TEXT("-%d.rgba"),I) : (IconOverride ? (I == 0 ? TEXT("-Default.rgba") : TEXT("-Override.rgba")) : (I == 0 ? TEXT("-Red.rgba") : TEXT("-Green.rgba")));
            const FString Path = FPaths::Combine(ArtifactDirectory, FString(ArtifactPrefix) + Suffix);
            if (Images[I].Num()!=4096 || FPaths::FileExists(Path)) { Error = TEXT("invalid or duplicate draw artifact"); return false; }
            TArray<uint8> Bytes;
            for (const FColor& Pixel : Images[I]) { Bytes.Add(Pixel.R); Bytes.Add(Pixel.G); Bytes.Add(Pixel.B); Bytes.Add(Pixel.A); }
            FSHA1::HashBuffer(Bytes.GetData(), Bytes.Num(), Hashes[I].Hash);
            if (!FFileHelper::SaveArrayToFile(Bytes, *Path)) { Error = TEXT("cannot write draw artifact"); return false; }
            if (ViewCases.Num() && !bColorDraw)
                CaseJson+=FString::Printf(TEXT("%s{\"game_time\":%.9g,\"real_time\":%.9g,\"sha1\":\"%s\"}"),I ? TEXT(",") : TEXT(""),ViewCases[I].Game,ViewCases[I].Real,*Hashes[I].ToString());
        }
        const FString MetadataPath = FPaths::Combine(ArtifactDirectory, FString(ArtifactPrefix) + TEXT(".json"));
        const FString Metadata = ViewCases.Num() && !bColorDraw ?
            FString::Printf(TEXT("{\"width\":64,\"height\":64,\"format\":\"RGBA8\",\"cases\":[%s],\"changed_pixels\":%d}\n"),*CaseJson,Different) :
            IconOverride ? FString::Printf(TEXT("{\"width\":64,\"height\":64,\"format\":\"RGBA8\",\"default_sha1\":\"%s\",\"override_sha1\":\"%s\",\"default_texture\":\"/Game/Athena/UI/Challenges/Art/Donut/T_UI_Donut_PC_Icon.T_UI_Donut_PC_Icon\",\"override_texture\":\"%s\",\"material_layout_hash\":%u,\"changed_pixels\":%d}\n"),*Hashes[0].ToString(),*Hashes[1].ToString(),*IconOverride->GetPathName(),Map->GetUniformExpressionSet().GetUniformBufferLayout().GetHash(),Different) :
            bColorDraw ? FString::Printf(TEXT("{\"width\":64,\"height\":64,\"format\":\"RGBA8\",\"game_time\":%.9g,\"real_time\":%.9g,\"red_sha1\":\"%s\",\"green_sha1\":\"%s\",\"changed_pixels\":%d}\n"),ViewCases[1].Game,ViewCases[1].Real,*Hashes[0].ToString(),*Hashes[1].ToString(),Different) :
            FString::Printf(TEXT("{\n  \"width\": 64, \"height\": 64, \"format\": \"RGBA8\",\n  \"red_sha1\": \"%s\", \"green_sha1\": \"%s\",\n  \"changed_pixels\": %d\n}\n"), *Hashes[0].ToString(), *Hashes[1].ToString(), Different);
        if (FPaths::FileExists(MetadataPath) || !FFileHelper::SaveStringToFile(Metadata, *MetadataPath))
        { Error = TEXT("cannot write draw metadata"); return false; }
    }
    if (!bPassed) { Error = Failure; return false; }
    return true;
}

static bool ParseLegacyMaterial(const TSharedPtr<FJsonObject>& Root, FLegacyMaterialIR& IR, FString& Error)
{
    const auto Material = Root->GetObjectField(TEXT("material"));
    const auto& Nodes = Root->GetArrayField(TEXT("nodes"));
    if (!Material || Nodes.Num() == 0 || Nodes.Num() > 4096) { Error = TEXT("invalid material fixture"); return false; }
    const FString MapHash = Material->GetStringField(TEXT("cooked_shader_map_id_hash"));
    if (MapHash.Len() != 40) { Error = TEXT("invalid material hash"); return false; }
    for (int32 I = 0; I < 20; ++I)
    {
        const int32 Hi = FParse::HexDigit(MapHash[I * 2]), Lo = FParse::HexDigit(MapHash[I * 2 + 1]);
        if (Hi < 0 || Lo < 0) { Error = TEXT("invalid material hash"); return false; }
        IR.OriginalMapHash.Hash[I] = (Hi << 4) | Lo;
    }
    IR.DebugDescription = Material->GetStringField(TEXT("friendly_name"));
    IR.QualityLevel = static_cast<EMaterialQualityLevel::Type>(int32(Material->GetNumberField(TEXT("quality_level"))));
    IR.FeatureLevel = static_cast<ERHIFeatureLevel::Type>(int32(Material->GetNumberField(TEXT("feature_level"))));
    if (IR.QualityLevel != EMaterialQualityLevel::High || IR.FeatureLevel != ERHIFeatureLevel::SM5)
    { Error = TEXT("unexpected quality or feature level"); return false; }
    const auto Expected = Root->GetObjectField(TEXT("expected_layout"));
    if (!Expected) { Error = TEXT("missing original layout"); return false; }
    const auto& Offsets=Expected->GetArrayField(TEXT("resource_offsets"));
    const auto& Types=Expected->GetArrayField(TEXT("resource_types"));
    if (Offsets.Num()<2 || Offsets.Num()!=Types.Num() || Offsets.Num()>258)
    { Error=TEXT("invalid original layout"); return false; }
    const int32 TotalSize=int32(Expected->GetNumberField(TEXT("constant_bytes")));
    const int32 NumericSize=int32(Offsets[0]->AsNumber());
    if (NumericSize<=0 || NumericSize>4096 || NumericSize%16 || TotalSize!=NumericSize+8*Offsets.Num())
    { Error=TEXT("invalid original layout size"); return false; }
    IR.UniformBufferSize=TotalSize;
    for (int32 I=0; I<Offsets.Num(); ++I)
    {
        const int32 Type=int32(Types[I]->AsNumber());
        if (int32(Offsets[I]->AsNumber())!=NumericSize+8*I || (Type!=UBMT_TEXTURE && Type!=UBMT_SAMPLER))
        { Error=TEXT("invalid original layout resource"); return false; }
        IR.Resources.Add({uint16(NumericSize+8*I), static_cast<EUniformBufferBaseType>(Type)});
    }
    const TArray<TSharedPtr<FJsonValue>>* Textures=nullptr;
    if (Root->TryGetArrayField(TEXT("texture_2d_parameters"),Textures))
    {
        if (Textures->Num()>128) { Error=TEXT("too many texture parameters"); return false; }
        for (const auto& Value : *Textures)
        {
            const auto T=Value->AsObject(); const auto P=T->GetObjectField(TEXT("parameter"));
            const auto Name=P->GetObjectField(TEXT("name"));
            const int32 Number=int32(Name->GetNumberField(TEXT("number")));
            const int32 Association=int32(P->GetNumberField(TEXT("association")));
            const int32 Index=int32(P->GetNumberField(TEXT("index")));
            auto& Out=IR.Texture2DParameters.AddDefaulted_GetRef();
            Out.Parameter=FMaterialParameterInfo(FName(*Name->GetStringField(TEXT("name")),Number),static_cast<EMaterialParameterAssociation>(Association),Index);
            Out.TextureIndex=int32(T->GetNumberField(TEXT("texture_index")));
            Out.TextureLayerIndex=int16(T->GetNumberField(TEXT("texture_layer_index")));
            Out.PageTableLayerIndex=int16(T->GetNumberField(TEXT("page_table_layer_index")));
            Out.SamplerSource=uint8(T->GetNumberField(TEXT("sampler_source")));
            Out.bVirtualTexture=T->GetBoolField(TEXT("virtual_texture"));
            if (Number<0 || Association<0 || Association>2 || Index< -1 || Out.TextureIndex<0 ||
                Out.TextureLayerIndex!= -1 || Out.PageTableLayerIndex!= -1 || Out.SamplerSource>2 || Out.bVirtualTexture)
            { Error=TEXT("unsupported texture parameter"); return false; }
        }
    }
    for (const auto& NodeValue : Nodes)
    {
        const auto Node = NodeValue->AsObject();
        if (!Node) { Error = TEXT("invalid expression node"); return false; }
        FLegacyMaterialExpression E;
        const FString Kind = Node->GetStringField(TEXT("type"));
        if (Kind == TEXT("Constant")) E.Kind = ELegacyMaterialExpression::Constant;
        else if (Kind == TEXT("ScalarParameter")) E.Kind = ELegacyMaterialExpression::ScalarParameter;
        else if (Kind == TEXT("VectorParameter")) E.Kind = ELegacyMaterialExpression::VectorParameter;
        else if (Kind == TEXT("FoldedMath")) E.Kind = ELegacyMaterialExpression::FoldedMath;
        else if (Kind == TEXT("AppendVector")) E.Kind = ELegacyMaterialExpression::AppendVector;
        else if (Kind == TEXT("ComponentSwizzle")) E.Kind = ELegacyMaterialExpression::ComponentSwizzle;
        else if (Kind == TEXT("TrigMath")) E.Kind = ELegacyMaterialExpression::TrigMath;
        else { Error = FString::Printf(TEXT("unsupported expression %s"), *Kind); return false; }
        const TArray<TSharedPtr<FJsonValue>>* Inputs = nullptr;
        if (Node->TryGetArrayField(TEXT("inputs"), Inputs))
        {
            if (Inputs->Num() > 3) { Error = TEXT("too many expression children"); return false; }
            int32* Dest[] = {&E.A, &E.B, &E.C};
            for (int32 I = 0; I < Inputs->Num(); ++I)
            {
                *Dest[I] = int32((*Inputs)[I]->AsNumber());
                if (*Dest[I] < 0 || *Dest[I] >= IR.Expressions.Num()) { Error = TEXT("invalid expression child"); return false; }
            }
        }
        double N;
        if (Node->TryGetNumberField(TEXT("value_type"), N)) E.ValueType = uint32(N);
        if (Node->TryGetNumberField(TEXT("operation"), N)) E.Operation = uint8(N);
        if (Node->TryGetNumberField(TEXT("components_a"), N)) E.Components = uint8(N);
        if (Node->TryGetNumberField(TEXT("elements"), N)) E.Components = uint8(N);
        const TArray<TSharedPtr<FJsonValue>>* Array = nullptr;
        if (Node->TryGetArrayField(TEXT("channels"), Array))
        {
            if (Array->Num() != 4) { Error = TEXT("invalid swizzle"); return false; }
            for (int32 I = 0; I < 4; ++I) E.Swizzle[I] = int8((*Array)[I]->AsNumber());
        }
        if (E.Kind == ELegacyMaterialExpression::Constant || E.Kind == ELegacyMaterialExpression::VectorParameter)
        {
            const TCHAR* Field = E.Kind == ELegacyMaterialExpression::Constant ? TEXT("value") : TEXT("default");
            if (!Node->TryGetArrayField(Field, Array) || Array->Num() != 4) { Error = TEXT("invalid vector value"); return false; }
            E.Value = FLinearColor((*Array)[0]->AsNumber(), (*Array)[1]->AsNumber(), (*Array)[2]->AsNumber(), (*Array)[3]->AsNumber());
        }
        else if (E.Kind == ELegacyMaterialExpression::ScalarParameter)
            E.Value.R = Node->GetNumberField(TEXT("default"));
        if (E.Kind == ELegacyMaterialExpression::ScalarParameter || E.Kind == ELegacyMaterialExpression::VectorParameter)
        {
            const auto P = Node->GetObjectField(TEXT("parameter"));
            const auto Name = P->GetObjectField(TEXT("name"));
            const int32 Number = int32(Name->GetNumberField(TEXT("number")));
            const int32 Association = int32(P->GetNumberField(TEXT("association")));
            const int32 Index = int32(P->GetNumberField(TEXT("index")));
            if (Number < 0 || Association < 0 || Association > 2 || Index < -1) { Error = TEXT("invalid parameter identity"); return false; }
            E.Parameter = FMaterialParameterInfo(FName(*Name->GetStringField(TEXT("name")), Number), static_cast<EMaterialParameterAssociation>(Association), Index);
        }
        IR.Expressions.Add(MoveTemp(E));
    }
    for (int32 Which = 0; Which < 2; ++Which)
    {
        const auto& Roots = Root->GetArrayField(Which == 0 ? TEXT("vector_roots") : TEXT("scalar_roots"));
        if (Roots.Num() > 256) { Error = TEXT("unexpected root count"); return false; }
        for (const auto& RootValue : Roots)
        {
            int32 Index = int32(RootValue->AsNumber());
            if (!IR.Expressions.IsValidIndex(Index)) { Error = TEXT("invalid root"); return false; }
            (Which == 0 ? IR.VectorRoots : IR.ScalarRoots).Add(Index);
        }
    }
    if (IR.VectorRoots.Num() * 16 + ((IR.ScalarRoots.Num() + 3) / 4) * 16 != NumericSize)
    { Error = TEXT("root counts do not match numeric layout"); return false; }
    return true;
}
// Registered before asset loading; this callback performs CPU-only conversion.
struct FLegacyIngressPin
{
    FString Owner, ShaderIdentity, BundleHash, ResourceHash, Json;
};
static TArray<FLegacyIngressPin> GLegacyIngressPins;
static bool GLegacyIngressConfigInvalid = false;
static const TCHAR* GLegacyDonutPath = TEXT("/Game/Athena/UI/Challenges/Art/Donut/M_UI_DonutIcons.M_UI_DonutIcons");
static const TCHAR* GLegacyMenuPath = TEXT("/Game/UI/Foundation/Materials/M_MenuButton_Master.M_MenuButton_Master");
static const TCHAR* GLegacyGradientPath = TEXT("/Game/UI/Foundation/Materials/M_UI_GradientPanel.M_UI_GradientPanel");
static const TCHAR* GLegacyPowerToastPath = TEXT("/Game/UI/Foundation/Border/Materials/M-Border-PowerToastGlow.M-Border-PowerToastGlow");
static const TCHAR* GLegacyLockerPath = TEXT("/Game/UI/Foundation/Materials/Athena_LockerTabMaster.Athena_LockerTabMaster");
struct FLegacyIngressProvenance final : FLegacyMaterialAssetProvenance
{
    FLegacyOriginalMaterial Decoded;
    FString BundleJson;
    int32 ArchiveUE4Version = 0, ArchiveLicenseeVersion = 0;
    virtual bool PrepareOwner(UMaterialInterface* Owner, FString& Error) const override
    {
        if (SourceIdentity!=GLegacyDonutPath) return true;
        UMaterial* Base=Owner ? Owner->GetMaterial() : nullptr;
        if (!IsInGameThread() || !Base || Base->GetPathName()!=SourceIdentity || Decoded.IR.Texture2DParameters.Num()!=1 ||
            Decoded.IR.Texture2DParameters[0].TextureIndex!=0)
        { Error=TEXT("invalid original Donut texture owner"); return false; }
        TSharedPtr<FJsonObject> Bundle;
        if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(BundleJson),Bundle) || !Bundle)
        { Error=TEXT("invalid original Donut texture provenance"); return false; }
        const auto Fixture=Bundle->GetObjectField(TEXT("runtime_fixture"));
        const auto& Textures=Fixture->GetArrayField(TEXT("texture_2d_parameters"));
        const FString Expected=TEXT("/Game/Athena/UI/Challenges/Art/Donut/T_UI_Donut_PC_Icon.T_UI_Donut_PC_Icon");
        if (Textures.Num()!=1 || Textures[0]->AsObject()->GetStringField(TEXT("default_texture_path"))!=Expected)
        { Error=TEXT("invalid original Donut indexed texture"); return false; }
        UTexture* Texture=LoadObject<UTexture>(nullptr,*Expected);
        if (!Texture || Texture->GetPathName()!=Expected)
        { Error=TEXT("original Donut texture could not load"); return false; }
        TArray<UTexture*> References; References.Add(Texture);
        return RestoreLegacyMaterialTextureReferences(Base,References,Error);
    }
};

static bool SameOriginalExpressions(const FLegacyMaterialIR& A, const FLegacyMaterialIR& B)
{
    if (A.OriginalMapHash != B.OriginalMapHash || A.QualityLevel != B.QualityLevel || A.FeatureLevel != B.FeatureLevel ||
        A.VectorRoots != B.VectorRoots || A.ScalarRoots != B.ScalarRoots || A.Expressions.Num() != B.Expressions.Num() || A.Texture2DParameters.Num()!=B.Texture2DParameters.Num()) return false;
    for (int32 I=0; I<A.Texture2DParameters.Num(); ++I)
    {
        const auto& X=A.Texture2DParameters[I]; const auto& Y=B.Texture2DParameters[I];
        if (X.Parameter!=Y.Parameter || X.TextureIndex!=Y.TextureIndex || X.TextureLayerIndex!=Y.TextureLayerIndex || X.PageTableLayerIndex!=Y.PageTableLayerIndex || X.SamplerSource!=Y.SamplerSource || X.bVirtualTexture!=Y.bVirtualTexture) return false;
    }
    for (int32 I=0; I<A.Expressions.Num(); ++I)
    {
        const auto& X=A.Expressions[I]; const auto& Y=B.Expressions[I];
        if (X.Kind!=Y.Kind || X.A!=Y.A || X.B!=Y.B || X.C!=Y.C || X.Value!=Y.Value ||
            X.Parameter!=Y.Parameter || X.ValueType!=Y.ValueType || X.Operation!=Y.Operation || X.Components!=Y.Components ||
            FMemory::Memcmp(X.Swizzle,Y.Swizzle,sizeof(X.Swizzle))) return false;
    }
    return true;
}

static ELegacyMaterialAssetResult ProvideLegacyOriginal(const FLegacyMaterialAssetContext& Context,
    TRefCountPtr<FMaterialShaderMap>& Map, TSharedPtr<const FLegacyMaterialAssetProvenance, ESPMode::ThreadSafe>& Provenance, FString& Error)
{
    const FLegacyIngressPin* Pin = nullptr;
    for (const auto& Candidate : GLegacyIngressPins)
        if (Candidate.Owner == Context.OwnerPath) { Pin = &Candidate; break; }
    if (!Pin) return ELegacyMaterialAssetResult::NotApplicable;
    auto Fail = [&Error](const TCHAR* Why) { Error=Why; return ELegacyMaterialAssetResult::Failed; };
    if (GLegacyIngressConfigInvalid || Pin->Json.IsEmpty()) return Fail(TEXT("invalid pinned ingress configuration"));
    TSharedPtr<FJsonObject> Bundle;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Pin->Json),Bundle) || !Bundle)
        return Fail(TEXT("cannot parse pinned bundle"));
    const auto Framing=Bundle->GetObjectField(TEXT("framing"));
    if (!Framing || Context.ResourceEnd-Context.ResourceBegin != int64(Framing->GetNumberField(TEXT("resource_bytes"))) ||
        Context.Archive.Tell()!=Context.ResourceBegin+8) return Fail(TEXT("Locker resource bounds mismatch"));
    TSharedRef<FLegacyIngressProvenance, ESPMode::ThreadSafe> P=MakeShared<FLegacyIngressProvenance, ESPMode::ThreadSafe>();
    P->ArchiveUE4Version=Context.Archive.UE4Ver();
    P->ArchiveLicenseeVersion=Context.Archive.LicenseeUE4Ver();
    UE_LOG(LogTemp, Display, TEXT("Legacy material ingress: %s archive UE4=%d licensee=%d"), *Context.OwnerPath, P->ArchiveUE4Version, P->ArchiveLicenseeVersion);
    P->Decoded.Bytes.SetNumUninitialized(int32(Context.ResourceEnd-Context.ResourceBegin));
    P->Decoded.Names.Append(Context.Names.GetData(), Context.Names.Num());
    const auto& ExpectedNames=Framing->GetArrayField(TEXT("names"));
    if (ExpectedNames.Num()!=P->Decoded.Names.Num()) return Fail(TEXT("Locker local name count mismatch"));
    for (int32 I=0; I<ExpectedNames.Num(); ++I)
        if (!ExpectedNames[I]->AsString().Equals(P->Decoded.Names[I].ToString(), ESearchCase::IgnoreCase)) return Fail(TEXT("Locker local name mismatch"));
    Context.Archive.Seek(Context.ResourceBegin);
    Context.Archive.Serialize(P->Decoded.Bytes.GetData(),P->Decoded.Bytes.Num());
    if (Context.Archive.IsError()) return Fail(TEXT("cannot read Locker resource"));
    const FString Hex=Bundle->GetStringField(TEXT("resource_raw_hex"));
    if (Hex.Len()!=P->Decoded.Bytes.Num()*2) return Fail(TEXT("Locker resource receipt length mismatch"));
    for(int32 I=0; I<P->Decoded.Bytes.Num(); ++I)
    {
        int32 Hi=FParse::HexDigit(Hex[I*2]), Lo=FParse::HexDigit(Hex[I*2+1]);
        if(Hi<0 || Lo<0 || P->Decoded.Bytes[I] != uint8((Hi<<4)|Lo)) return Fail(TEXT("Locker resource differs from pinned original"));
    }
    FSHAHash OriginalHash; FSHA1::HashBuffer(P->Decoded.Bytes.GetData(),P->Decoded.Bytes.Num(),OriginalHash.Hash);
    if (!OriginalHash.ToString().Equals(Pin->ResourceHash,ESearchCase::IgnoreCase))
        return Fail(TEXT("Locker source digest mismatch"));
    if (!DecodeLegacyOriginalMaterial(P->Decoded,Error)) return ELegacyMaterialAssetResult::Failed;
    if (P->Decoded.MaterialPath != Pin->ShaderIdentity || P->Decoded.Records.Num()!=19) return Fail(TEXT("Locker decoded identity mismatch"));
    // The library receipt covers all source records, including LocalVF records
    // that this Slate-only runtime factory does not convert.
    const auto Report=Bundle->GetObjectField(TEXT("decoded"));
    if (!Report || int32(Report->GetNumberField(TEXT("shader_record_count")))!=19) return Fail(TEXT("invalid complete shader report"));
    TArray<TSharedPtr<FJsonValue>> Rows=Report->GetObjectField(TEXT("material_shaders"))->GetArrayField(TEXT("records"));
    for (const auto& Mesh : Report->GetArrayField(TEXT("mesh_shader_maps")))
        Rows.Append(Mesh->AsObject()->GetArrayField(TEXT("records")));
    if (Rows.Num()!=P->Decoded.Records.Num()) return Fail(TEXT("original shader receipt count mismatch"));
    for (int32 I=0; I<Rows.Num(); ++I)
    {
        const auto Row=Rows[I]->AsObject(); const auto Association=Row->GetObjectField(TEXT("library_association"));
        const auto& Record=P->Decoded.Records[I];
        if (!Association || Row->GetObjectField(TEXT("type"))->GetStringField(TEXT("name"))!=Record.Type ||
            int32(Row->GetNumberField(TEXT("offset")))!=Record.Begin || int32(Row->GetNumberField(TEXT("end")))!=Record.End ||
            !Row->GetObjectField(TEXT("output_hash"))->GetStringField(TEXT("hash")).Equals(Record.OutputHash.ToString(),ESearchCase::IgnoreCase) ||
            uint32(Association->GetNumberField(TEXT("frequency")))!=Record.Frequency ||
            uint32(Association->GetNumberField(TEXT("uncompressed_bytes")))!=Record.UncompressedBytes)
            return Fail(TEXT("original shader library association mismatch"));
        const int32 LibraryIndex=int32(Association->GetNumberField(TEXT("index")));
        if (LibraryIndex<0) return Fail(TEXT("invalid shader library index"));
    }
    const auto Fixture=Bundle->GetObjectField(TEXT("runtime_fixture"));
    FLegacyMaterialIR Expected;
    if (!Fixture || P->Decoded.IR.DebugDescription != Fixture->GetObjectField(TEXT("material"))->GetStringField(TEXT("debug_description")) ||
        !ParseLegacyMaterial(Fixture,Expected,Error) || !SameOriginalExpressions(P->Decoded.IR,Expected))
        return Fail(TEXT("decoded source graph differs from validated runtime fixture"));
    const auto& Shaders=Fixture->GetArrayField(TEXT("shaders"));
    if (Shaders.Num()!=7) return Fail(TEXT("invalid Slate fixture count"));
    for (int32 I=0; I<Shaders.Num(); ++I)
    {
        const auto Row=Shaders[I]->AsObject()->GetObjectField(TEXT("record"));
        if (!Row || Row->GetObjectField(TEXT("type"))->GetStringField(TEXT("name"))!=P->Decoded.Records[I].Type ||
            !Row->GetObjectField(TEXT("output_hash"))->GetStringField(TEXT("hash")).Equals(P->Decoded.Records[I].OutputHash.ToString(),ESearchCase::IgnoreCase) ||
            uint32(Row->GetNumberField(TEXT("frequency")))!=P->Decoded.Records[I].Frequency)
            return Fail(TEXT("Slate shader receipt does not match original record"));
    }
    P->Decoded.IR.UniformBufferSize=Expected.UniformBufferSize;
    P->Decoded.IR.Resources=Expected.Resources;
    Map=FMaterialShaderMap::CreateLegacySlateMap(P->Decoded.IR,
        [&Shaders, Pin](const FUniformExpressionSet& Uniforms, const FShaderParametersMetadata&, TArray<FLegacySlateShaderInput>& Outputs, FString& OutError)
        {
            for (const auto& Value : Shaders)
            {
                const auto Shader=Value->AsObject();
                if (!Shader || !PrepareLegacyOutput(Shader,Uniforms.GetUniformBufferLayout(),Outputs.AddDefaulted_GetRef(),OutError,false,Pin->Owner==GLegacyMenuPath)) return false;
            }
            return true;
        },Error);
    if (!Map.IsValid()) return ELegacyMaterialAssetResult::Failed;
    P->OriginalResource=P->Decoded.Bytes;
    FSHA1::HashBuffer(P->OriginalResource.GetData(),P->OriginalResource.Num(),P->ResourceHash.Hash);
    P->SourceIdentity=Pin->Owner; P->OriginalShaderRecordCount=P->Decoded.Records.Num(); P->BundleJson=Pin->Json;
    Provenance=P; return ELegacyMaterialAssetResult::Converted;
}
#endif

void RegisterLegacyOriginalMaterialProvider()
{
#if PLATFORM_LINUX && !UE_SERVER
    FString Path;
    if (!FParse::Value(FCommandLine::Get(),TEXT("FortniteLegacyMaterialIngress="),Path,false)) return;
    // Pinned entries are immutable after startup. Each invocation parses its own JSON.
    GLegacyIngressPins = {
        {GLegacyLockerPath, GLegacyLockerPath, TEXT("0f750204f6008e6e3219d0b002d8409e28c22426"), TEXT("bc2d2f509b6909a4820f65188b85485fce989a2a"), FString()},
        {GLegacyGradientPath, GLegacyGradientPath, TEXT("87617e2d5b3c8ee57d8ede04783abe2a32fafd74"), TEXT("a088433310c1bb0de8b63eb3096929fed602ba4f"), FString()},
        {GLegacyPowerToastPath, GLegacyGradientPath, TEXT("1aef96f25f926219b10ceb69c8f3731a3b118fa6"), TEXT("d8b5194ca11c187ffad72b17e9517ecf40fd1266"), FString()},
        {GLegacyMenuPath, GLegacyMenuPath, TEXT("0b49a605763b907dbdf1f0041715208a81d4744a"), TEXT("955e034f179ab1129d49286356357039e57260e4"), FString()},
        {GLegacyDonutPath, GLegacyDonutPath, TEXT("e794ad691e278ecf653dfef34c0ae07375f32d7d"), TEXT("2a27448730935ebe70c3a30e765ad6784a163668"), FString()}
    };
    TArray<FString> Paths;
    Path.ParseIntoArray(Paths, TEXT(","), true);
    for (const FString& BundlePath : Paths)
    {
        TArray<uint8> Bytes; FSHAHash Hash;
        if (!FFileHelper::LoadFileToArray(Bytes,*BundlePath) || Bytes.Num()>32*1024*1024)
        { GLegacyIngressConfigInvalid=true; break; }
        FSHA1::HashBuffer(Bytes.GetData(),Bytes.Num(),Hash.Hash);
        FLegacyIngressPin* Match=nullptr;
        for (auto& Pin : GLegacyIngressPins)
            if (Hash.ToString().Equals(Pin.BundleHash,ESearchCase::IgnoreCase)) { Match=&Pin; break; }
        if (!Match || !Match->Json.IsEmpty()) { GLegacyIngressConfigInvalid=true; break; }
        FFileHelper::BufferToString(Match->Json,Bytes.GetData(),Bytes.Num());
        TSharedPtr<FJsonObject> Bundle;
        if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Match->Json),Bundle) || !Bundle)
        { GLegacyIngressConfigInvalid=true; break; }
        const FString Schema=Bundle->GetStringField(TEXT("schema"));
        if ((Match->Owner==GLegacyLockerPath && Schema!=TEXT("fortnite.legacy-material-ingress.v1")) ||
            (Match->Owner!=GLegacyLockerPath && (Schema!=(Match->Owner==GLegacyDonutPath ? TEXT("fortnite.legacy-material-ingress.v3") : TEXT("fortnite.legacy-material-ingress.v2")) ||
             Bundle->GetStringField(TEXT("owner_path"))!=Match->Owner ||
             Bundle->GetStringField(TEXT("parent_path"))!=(Match->Owner==GLegacyPowerToastPath ? FString(GLegacyGradientPath) : FString()) ||
             Bundle->GetStringField(TEXT("owner_type"))!=(Match->Owner==GLegacyPowerToastPath ? TEXT("UMaterialInstanceConstant") : TEXT("UMaterial")))))
        { GLegacyIngressConfigInvalid=true; break; }
    }
    if (!Paths.Num()) GLegacyIngressConfigInvalid=true;
    FModuleManager::LoadModuleChecked<IModuleInterface>(TEXT("SlateRHIRenderer"));
    RegisterLegacyMaterialAssetProvider(&ProvideLegacyOriginal);
    UE_LOG(LogTemp,Display,TEXT("Legacy material ingress: registered pinned provider, configuration valid=%d"),!GLegacyIngressConfigInvalid);
#endif
}

void ProbeLegacyOriginalMaterial()
{
#if PLATFORM_LINUX && !UE_SERVER
    if (!FParse::Param(FCommandLine::Get(),TEXT("FortniteProbeLegacyOriginalMaterial"))) return;
    FString OwnerPath=GLegacyLockerPath;
    FParse::Value(FCommandLine::Get(),TEXT("FortniteLegacyMaterialProbeOwner="),OwnerPath);
    const FLegacyIngressPin* Pin=nullptr;
    for (const auto& Candidate : GLegacyIngressPins) if (Candidate.Owner==OwnerPath) { Pin=&Candidate; break; }
    TSharedPtr<FJsonObject> Bundle;
    if (!Pin || GLegacyIngressConfigInvalid || !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Pin->Json),Bundle) || !Bundle)
    { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: probe has no valid bundle")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    UMaterialInterface* Material=LoadObject<UMaterialInterface>(nullptr,*OwnerPath);
    if (OwnerPath == GLegacyPowerToastPath)
    {
        UMaterialInstance* Instance=Cast<UMaterialInstance>(Material);
        UMaterial* Base=Material ? Material->GetMaterial() : nullptr;
        const FString ParentPath=Bundle->GetStringField(TEXT("parent_path"));
        float Rotation=0.f, Scale=0.f, Bias=0.f;
        FLinearColor Color0, Color1;
        if (!Instance || !Instance->Parent || Instance->Parent->GetPathName()!=ParentPath || !Base ||
            Base->GetPathName()!=Pin->ShaderIdentity ||
            !Instance->GetScalarParameterValue(FHashedMaterialParameterInfo(TEXT("Rotation")),Rotation,true) || !FMath::IsNearlyEqual(Rotation,1.f) ||
            !Instance->GetScalarParameterValue(FHashedMaterialParameterInfo(TEXT("Scale")),Scale,true) || !FMath::IsNearlyEqual(Scale,1.5f) ||
            !Instance->GetScalarParameterValue(FHashedMaterialParameterInfo(TEXT("RefractionDepthBias")),Bias,true) || !FMath::IsNearlyZero(Bias) ||
            !Instance->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("RGBA0")),Color0,true) || !Color0.Equals(FLinearColor(.033105f,.246201f,1.f,0.f),1.e-5f) ||
            !Instance->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("RGBA1")),Color1,true) || !Color1.Equals(FLinearColor(.033105f,.246201f,1.f,1.f),1.e-5f))
        { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: real instance parent or original overrides differ")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
        const FLegacyIngressPin* ParentPin=nullptr;
        for (const auto& Candidate : GLegacyIngressPins) if (Candidate.Owner==ParentPath) { ParentPin=&Candidate; break; }
        FMaterialResource* ParentResource=Base->GetMaterialResource(GMaxRHIFeatureLevel);
        FMaterialShaderMap* ParentMap=ParentResource ? ParentResource->GetGameThreadShaderMap() : nullptr;
        const auto ParentProvenance=ParentMap ? ParentMap->GetLegacyMaterialProvenance() : nullptr;
        bool bParentRenderAttached=false;
        if (ParentResource && ParentMap)
        {
            ENQUEUE_RENDER_COMMAND(CheckOriginalParentAttachment)([ParentResource, ParentMap, &bParentRenderAttached](FRHICommandListImmediate&)
            { bParentRenderAttached=ParentResource->GetRenderingThreadShaderMap()==ParentMap; });
            FlushRenderingCommands();
        }
        if (!ParentPin || !ParentProvenance.IsValid() || ParentProvenance->SourceIdentity!=ParentPath ||
            ParentProvenance->OriginalShaderRecordCount!=19 ||
            !ParentProvenance->ResourceHash.ToString().Equals(ParentPin->ResourceHash,ESearchCase::IgnoreCase) ||
            !ParentMap->IsComplete(ParentResource,false) || !bParentRenderAttached)
        { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: real parent attachment or provenance failed")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
        UE_LOG(LogTemp,Display,TEXT("Legacy material ingress: real parent map complete and attached on game/render threads; instance overrides verified"));
    }
    FMaterialResource* Resource=Material ? Material->GetMaterialResource(GMaxRHIFeatureLevel) : nullptr;
    FMaterialShaderMap* Map=Resource ? Resource->GetGameThreadShaderMap() : nullptr;
    const auto Provenance=Map ? Map->GetLegacyMaterialProvenance() : nullptr;
    const bool bOriginal=Provenance.IsValid() && Provenance->OriginalShaderRecordCount==19 &&
        Provenance->SourceIdentity==OwnerPath &&
        Provenance->ResourceHash.ToString().Equals(Pin->ResourceHash,ESearchCase::IgnoreCase);
    const bool bComplete=Map && Resource && Map->IsComplete(Resource,false);
    bool bRenderAttached=false;
    if (bOriginal && bComplete)
    {
        ENQUEUE_RENDER_COMMAND(CheckOriginalMaterialAttachment)([Resource, Map, &bRenderAttached](FRHICommandListImmediate&)
        { bRenderAttached=Resource->GetRenderingThreadShaderMap()==Map; });
        FlushRenderingCommands();
    }
    if (!bOriginal || !bComplete || !bRenderAttached)
    {
        UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: original Locker attachment failed owner=%d map=%d provenance=%d complete=%d render=%d"),
            Material!=nullptr,Map!=nullptr,bOriginal,bComplete,bRenderAttached);
        FPlatformMisc::RequestExitWithStatus(false,1);
        return;
    }
    if (OwnerPath==GLegacyDonutPath)
    {
        const auto Referenced=Resource->GetReferencedTextures();
        const FString TexturePath=TEXT("/Game/Athena/UI/Challenges/Art/Donut/T_UI_Donut_PC_Icon.T_UI_Donut_PC_Icon");
        UTexture* Texture=Referenced.Num()>0 ? Cast<UTexture>(Referenced[0]) : nullptr;
        if (!Texture || Texture->GetPathName()!=TexturePath)
        { UE_LOG(LogTemp,Error,TEXT("Legacy Donut: original indexed texture missing or different")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
        bool bTextureMatches=false;
        const FMaterialRenderProxy* Base=Material->GetRenderProxy();
        ENQUEUE_RENDER_COMMAND(CheckLegacyDonutTexture)([Map,Resource,Base,Texture,&bTextureMatches](FRHICommandListImmediate&)
        {
            const FMaterialRenderProxy* Fallback=nullptr;
            const FMaterial& Selected=Base->GetMaterialWithFallback(ERHIFeatureLevel::SM5,Fallback);
            if (&Selected!=Resource || !Texture->Resource || !Texture->TextureReference.TextureReferenceRHI || !Texture->Resource->SamplerStateRHI ||
                !Texture->TextureReference.TextureReferenceRHI->GetReferencedTexture() || Texture->TextureReference.TextureReferenceRHI->GetReferencedTexture()==GWhiteTexture->TextureRHI.GetReference()) return;
            FLegacyProbeProxy Proxy; Proxy.Base=Base; Proxy.bInherit=true;
            FMaterialRenderContext Context(&Proxy,Selected,nullptr); FUniformExpressionCache Cache;
            alignas(16) uint8 Contents[4112]={};
            const auto& Uniforms=Map->GetUniformExpressionSet();
            Uniforms.FillUniformBuffer(Context,Cache,Contents,sizeof(Contents));
            const auto& Layout=Uniforms.GetUniformBufferLayout();
            if (Layout.Resources.Num()!=4 || Layout.Resources[0].MemberOffset!=96 || Layout.Resources[1].MemberOffset!=104) return;
            void* TextureRHI=nullptr; void* SamplerRHI=nullptr;
            FMemory::Memcpy(&TextureRHI,Contents+96,sizeof(void*)); FMemory::Memcpy(&SamplerRHI,Contents+104,sizeof(void*));
            bTextureMatches=TextureRHI==Texture->TextureReference.TextureReferenceRHI.GetReference() && SamplerRHI==Texture->Resource->SamplerStateRHI.GetReference();
        });
        FlushRenderingCommands();
        if (!bTextureMatches) { UE_LOG(LogTemp,Error,TEXT("Legacy Donut: actual texture/sampler uniform differs")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
        FString Directory;
        if (FParse::Value(FCommandLine::Get(),TEXT("FortniteLegacyMaterialArtifactDir="),Directory))
        {
            const auto& Layout=Map->GetUniformExpressionSet().GetUniformBufferLayout();
            FString Resources;
            for (int32 I=0; I<Layout.Resources.Num(); ++I)
                Resources+=FString::Printf(TEXT("%s{\"offset\":%u,\"type\":%u}"),I ? TEXT(",") : TEXT(""),uint32(Layout.Resources[I].MemberOffset),uint32(Layout.Resources[I].MemberType));
            const FString Json=FString::Printf(TEXT("{\"constant_bytes\":%u,\"static_slot\":%u,\"layout_hash\":%u,\"resources\":[%s],\"indexed_texture\":\"%s\"}\n"),uint32(Layout.ConstantBufferSize),uint32(Layout.StaticSlot),Layout.GetHash(),*Resources,*TexturePath);
            const FString Path=FPaths::Combine(Directory,TEXT("DonutMaterialLayout.json"));
            if (FPaths::FileExists(Path) || !FFileHelper::SaveStringToFile(Json,*Path))
            { UE_LOG(LogTemp,Error,TEXT("Legacy Donut: could not write material layout")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
        }
        UE_LOG(LogTemp,Display,TEXT("Legacy Donut: indexed source texture and actual uniform texture/sampler match %s"),*TexturePath);
    }
    const auto* Original=static_cast<const FLegacyIngressProvenance*>(Provenance.Get());
    auto RejectMutation = [&Original](const TCHAR* Case, TArray<uint8> Bytes) -> bool
    {
        FLegacyOriginalMaterial Test; Test.Bytes=MoveTemp(Bytes); Test.Names=Original->Decoded.Names;
        FString Error; const bool bRejected=!DecodeLegacyOriginalMaterial(Test,Error);
        UE_LOG(LogTemp,Display,TEXT("Legacy material ingress reader negative %s: %s"),Case,bRejected ? TEXT("rejected") : TEXT("ACCEPTED"));
        return bRejected;
    };
    const auto& Bytes=Original->Decoded.Bytes;
    TArray<uint8> Truncated=Bytes; Truncated.SetNum(Original->Decoded.Records[0].Begin+24);
    TArray<uint8> Appended=Bytes; Appended.Add(0);
    TArray<uint8> BadEnd=Bytes; const int64 EscapingEnd=int64(Bytes.Num())+1;
    FMemory::Memcpy(BadEnd.GetData()+Original->Decoded.Records[0].Begin+8,&EscapingEnd,sizeof(EscapingEnd));
    TArray<uint8> BadName=Bytes; const int32 InvalidName=MAX_int32;
    const int32 FirstExpression=int32(Bundle->GetObjectField(TEXT("decoded"))->GetObjectField(TEXT("uniform_expression_arrays"))->GetObjectField(TEXT("vector"))->GetNumberField(TEXT("offset")))+4;
    if (!BadName.IsValidIndex(FirstExpression+3)) { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: invalid mutation offset")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    FMemory::Memcpy(BadName.GetData()+FirstExpression,&InvalidName,sizeof(InvalidName));
    bool bNegativeChecks=true;
    bNegativeChecks &= RejectMutation(TEXT("truncated shader record"),MoveTemp(Truncated));
    bNegativeChecks &= RejectMutation(TEXT("trailing byte"),MoveTemp(Appended));
    bNegativeChecks &= RejectMutation(TEXT("shader end escapes resource"),MoveTemp(BadEnd));
    bNegativeChecks &= RejectMutation(TEXT("invalid local expression name"),MoveTemp(BadName));
    if (!bNegativeChecks)
    { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: original reader negative check failed")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    FString Error;
    if (!CheckLegacyUniforms(Map->GetUniformExpressionSet(),Bundle->GetObjectField(TEXT("runtime_fixture")),TEXT("owner"),Error,OwnerPath==GLegacyDonutPath ? Material : nullptr))
    { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: live uniform cases failed: %s"),*Error); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    if (OwnerPath == GLegacyPowerToastPath && !CheckLegacyUniforms(Map->GetUniformExpressionSet(),Bundle->GetObjectField(TEXT("runtime_fixture")),TEXT("instance"),Error,Material,TEXT("owner_uniform_cases")))
    { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: live instance uniform cases failed: %s"),*Error); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    if (OwnerPath==GLegacyDonutPath && !CheckLegacyUniforms(Map->GetUniformExpressionSet(),Bundle->GetObjectField(TEXT("runtime_fixture")),TEXT("Donut owner"),Error,Material,TEXT("owner_uniform_cases")))
    { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: Donut owner uniform cases failed: %s"),*Error); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    if (OwnerPath==GLegacyMenuPath && !CheckLegacyUniforms(Map->GetUniformExpressionSet(),Bundle->GetObjectField(TEXT("runtime_fixture")),TEXT("View draw"),Error,nullptr,TEXT("view_cases")))
    { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: View draw uniforms failed: %s"),*Error); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    if (OwnerPath==GLegacyMenuPath)
    {
        const FShaderParametersMetadata* Metadata=FViewUniformShaderParameters::FTypeInfo::GetStructMetadata();
        const auto& Layout=Metadata->GetLayout();
        FString Fields, Resources, Directory;
        int32 FieldCount=0;
        for (const auto& Field : Metadata->GetMembers())
        {
            if (FCString::Strcmp(Field.GetName(),TEXT("GameTime")) && FCString::Strcmp(Field.GetName(),TEXT("RealTime"))) continue;
            Fields+=FString::Printf(TEXT("%s{\"name\":\"%s\",\"offset\":%u,\"type\":%u,\"size\":%u}"),FieldCount++ ? TEXT(",") : TEXT(""),Field.GetName(),Field.GetOffset(),uint32(Field.GetBaseType()),Field.GetMemberSize());
        }
        for (int32 I=0; I<Layout.Resources.Num(); ++I)
            Resources+=FString::Printf(TEXT("%s{\"offset\":%u,\"type\":%u}"),I ? TEXT(",") : TEXT(""),uint32(Layout.Resources[I].MemberOffset),uint32(Layout.Resources[I].MemberType));
        if (FieldCount!=2) { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: View fields missing")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
        if (FParse::Value(FCommandLine::Get(),TEXT("FortniteLegacyMaterialArtifactDir="),Directory))
        {
            const FString Path=FPaths::Combine(Directory,TEXT("GTViewLayout.json"));
            const FString Json=FString::Printf(TEXT("{\"constant_bytes\":%u,\"static_slot\":%u,\"layout_hash\":%u,\"game_time_offset\":%u,\"real_time_offset\":%u,\"resources\":[%s],\"fields\":[%s]}\n"),uint32(Layout.ConstantBufferSize),uint32(Layout.StaticSlot),Layout.GetHash(),uint32(STRUCT_OFFSET(FViewUniformShaderParameters,GameTime)),uint32(STRUCT_OFFSET(FViewUniformShaderParameters,RealTime)),*Resources,*Fields);
            if (FPaths::FileExists(Path) || !FFileHelper::SaveStringToFile(Json,*Path))
            { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: cannot write View layout receipt")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
        }
    }
    FLegacyDrawBindings Bindings; bool bVS=false, bPS=false, bView=OwnerPath!=GLegacyMenuPath, bAllShaders=true; int32 CheckedShaders=0;
    const auto Fixture=Bundle->GetObjectField(TEXT("runtime_fixture"));
    for (const auto& Value : Fixture->GetArrayField(TEXT("shaders")))
    {
        const auto Shader=Value->AsObject(); FLegacySlateShaderInput Input;
        if (!PrepareLegacyOutput(Shader,Map->GetUniformExpressionSet().GetUniformBufferLayout(),Input,Error,true,OwnerPath==GLegacyMenuPath)) { bAllShaders=false; break; }
        FShaderType* Type=FindShaderTypeByName(FHashedName(*Input.TypeName));
        const TShaderRef<FShader> Frozen=Type ? Map->GetShader(Type,Input.PermutationId) : TShaderRef<FShader>();
        const FShaderMapResourceCode* Code=Map->GetResourceCode();
        const int32 Index=Frozen.IsValid() ? Frozen->GetResourceIndex() : INDEX_NONE;
        const int32 Frequency=int32(Shader->GetObjectField(TEXT("record"))->GetNumberField(TEXT("frequency")));
        if (!Code || !Code->ShaderHashes.IsValidIndex(Index) || !Code->ShaderEntries.IsValidIndex(Index) ||
            Code->ShaderHashes[Index]!=Input.Output.OutputHash || int32(Code->ShaderEntries[Index].Frequency)!=Frequency)
        { Error=TEXT("attached frozen shader resource mismatch"); bAllShaders=false; break; }
        ++CheckedShaders;
        uint16 Size=0, Base=0;
        if (Input.TypeName==TEXT("TSlateMaterialShaderVSfalse"))
            bVS=Input.Output.ParameterMap.FindParameterAllocation(TEXT("ViewProjection"),Bindings.MatrixBuffer,Bindings.MatrixOffset,Size) && Size==sizeof(FMatrix);
        if (Input.TypeName==TEXT("TSlateMaterialShaderPSDefault"))
            bPS=Input.Output.ParameterMap.FindParameterAllocation(TEXT("GammaAndAlphaValues"),Bindings.GammaBuffer,Bindings.GammaOffset,Size) && Size==sizeof(FVector4) &&
                Input.Output.ParameterMap.FindParameterAllocation(TEXT("ShaderParams"),Bindings.ParamsBuffer,Bindings.ParamsOffset,Size) && Size==sizeof(FVector4) &&
                Input.Output.ParameterMap.FindParameterAllocation(TEXT("Material"),Bindings.MaterialBuffer,Base,Size) && Base==FVulkanShaderHeader::UniformBuffer && Size==1;
        if (OwnerPath==GLegacyMenuPath && Input.TypeName==TEXT("TSlateMaterialShaderPSDefault"))
            bView=Input.Output.ParameterMap.FindParameterAllocation(TEXT("View"),Bindings.ViewBuffer,Base,Size) && Base==FVulkanShaderHeader::UniformBuffer && Size==1;
    }
    UTexture* IconOverride=nullptr;
    if (OwnerPath==GLegacyDonutPath)
    {
        UMaterialInstance* Instance=LoadObject<UMaterialInstance>(nullptr,TEXT("/Game/Athena/UI/Challenges/Art/Donut/M_UI_DonutIcons_App2.M_UI_DonutIcons_App2"));
        if (!Instance || Instance->Parent!=Material ||
            !Instance->GetTextureParameterValue(FHashedMaterialParameterInfo(TEXT("Icon")),IconOverride,true) || !IconOverride ||
            IconOverride->GetPathName()!=TEXT("/Game/Athena/UI/Challenges/Art/Donut/T_UI_Donut_App_Icon.T_UI_Donut_App_Icon"))
        { UE_LOG(LogTemp,Error,TEXT("Legacy Donut: actual App2 Icon override differs")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    }
    if (!bAllShaders || CheckedShaders!=7 || !bVS || !bPS || !bView || !DrawLegacySlateQuad(Map,Bindings,Error,Material,Resource,OwnerPath==GLegacyLockerPath ? TEXT("RBGA1") : TEXT("RGBA1"),OwnerPath==GLegacyLockerPath ? TEXT("LockerQuad") : OwnerPath==GLegacyMenuPath ? TEXT("MenuQuad") : OwnerPath==GLegacyDonutPath ? TEXT("DonutQuad") : TEXT("PowerToastQuad"),OwnerPath==GLegacyMenuPath ? Fixture : nullptr,false,IconOverride))
    {
        UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: attached owner draw failed: %s"),*Error);
        FPlatformMisc::RequestExitWithStatus(false,1); return;
    }
    if (OwnerPath==GLegacyMenuPath &&
        (!Fixture->GetStringField(TEXT("draw_parameter")).Equals(TEXT("Primary Color")) ||
         !DrawLegacySlateQuad(Map,Bindings,Error,Material,Resource,TEXT("Primary Color"),TEXT("MenuColorQuad"),Fixture,true)))
    { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: Menu color draw failed: %s"),*Error); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    UE_LOG(LogTemp,Display,TEXT("Legacy material ingress: original owner attached complete map with 19 original records on game and render threads and drew its Slate quad"));
#endif
}

void RunLegacyMaterialShaderMapProbe()
{
#if PLATFORM_LINUX && !UE_SERVER
    FString FixturePath;
    if (!FParse::Value(FCommandLine::Get(), TEXT("FortniteLegacyMaterialFixture="), FixturePath)) return;
    if (!GDynamicRHI || FCString::Strcmp(GDynamicRHI->GetName(), TEXT("Vulkan")) != 0 || GMaxRHIShaderPlatform != SP_VULKAN_SM5)
    { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: Vulkan SM5 RHI required")); return; }
    FString Text, Error;
    TArray<uint8> FixtureBytes;
    FSHAHash FixtureHash;
    if (!FFileHelper::LoadFileToArray(FixtureBytes, *FixturePath) || FixtureBytes.Num() > 8 * 1024 * 1024)
    { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: cannot read fixture")); return; }
    FSHA1::HashBuffer(FixtureBytes.GetData(), FixtureBytes.Num(), FixtureHash.Hash);
    if (!FixtureHash.ToString().Equals(TEXT("3c379d3b0b97bb8e57e9b7b8b7286182ac78a700"), ESearchCase::IgnoreCase))
    { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: fixture digest mismatch")); return; }
    FFileHelper::BufferToString(Text, FixtureBytes.GetData(), FixtureBytes.Num());
    TSharedPtr<FJsonObject> Root;
    if (Text.Len() > 8 * 1024 * 1024 ||
        !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root) || !Root ||
        Root->GetStringField(TEXT("schema")) != TEXT("fortnite.legacy-slate.v1"))
    { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: invalid fixture")); return; }
    FLegacyMaterialIR IR;
    if (!ParseLegacyMaterial(Root, IR, Error))
    { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: %s"), *Error); return; }
    const auto& Shaders = Root->GetArrayField(TEXT("shaders"));
    if (Shaders.Num() != 7)
    { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: expected seven shaders")); return; }
    FModuleManager::LoadModuleChecked<IModuleInterface>(TEXT("SlateRHIRenderer"));
    TArray<FSHAHash> OutputHashes;
    FLegacyDrawBindings DrawBindings;
    bool bHaveVSBindings = false, bHavePSBindings = false;
    TRefCountPtr<FMaterialShaderMap> Map = FMaterialShaderMap::CreateLegacySlateMap(IR,
        [&Shaders, &Root, &OutputHashes, &DrawBindings, &bHaveVSBindings, &bHavePSBindings](const FUniformExpressionSet& Uniforms, const FShaderParametersMetadata&, TArray<FLegacySlateShaderInput>& Outputs, FString& OutError)
        {
            if (!CheckLegacyUniforms(Uniforms, Root, TEXT("pre-freeze"), OutError)) return false;
            const FRHIUniformBufferLayout& Layout = Uniforms.GetUniformBufferLayout();
            for (const auto& Value : Shaders)
            {
                const auto Record = Value->AsObject();
                if (!Record) { OutError = TEXT("invalid shader record"); return false; }
                FLegacySlateShaderInput& Input = Outputs.AddDefaulted_GetRef();
                if (!PrepareLegacyOutput(Record, Layout, Input, OutError)) return false;
                OutputHashes.Add(Input.Output.OutputHash);
                uint16 Size = 0, Base = 0;
                if (Input.TypeName == TEXT("TSlateMaterialShaderVSfalse"))
                {
                    bHaveVSBindings = Input.Output.ParameterMap.FindParameterAllocation(TEXT("ViewProjection"), DrawBindings.MatrixBuffer, DrawBindings.MatrixOffset, Size) && Size == sizeof(FMatrix);
                }
                if (Input.TypeName == TEXT("TSlateMaterialShaderPSDefault"))
                {
                    bHavePSBindings = Input.Output.ParameterMap.FindParameterAllocation(TEXT("GammaAndAlphaValues"), DrawBindings.GammaBuffer, DrawBindings.GammaOffset, Size) && Size == sizeof(FVector4) &&
                        Input.Output.ParameterMap.FindParameterAllocation(TEXT("ShaderParams"), DrawBindings.ParamsBuffer, DrawBindings.ParamsOffset, Size) && Size == sizeof(FVector4) &&
                        Input.Output.ParameterMap.FindParameterAllocation(TEXT("Material"), DrawBindings.MaterialBuffer, Base, Size) && Base == FVulkanShaderHeader::UniformBuffer && Size == 1;
                }
            }
            return true;
        }, Error);
    if (!Map.IsValid())
    { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: %s"), *Error); return; }
    if (!CheckLegacyUniforms(Map->GetUniformExpressionSet(), Root, TEXT("post-freeze"), Error))
    { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: %s"), *Error); return; }
    TArray<FShaderType*> Types;
    int32 ShaderNumber = 0;
    for (const auto& Shader : Shaders)
    {
        const auto Record = Shader->AsObject()->GetObjectField(TEXT("record"));
        FShaderType* Type = FindShaderTypeByName(FHashedName(*Record->GetObjectField(TEXT("type"))->GetStringField(TEXT("name"))));
        if (!Type || !Map->GetShader(Type, 0).IsValid())
        { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: frozen shader lookup failed")); return; }
        const TShaderRef<FShader> Frozen = Map->GetShader(Type, 0);
        const int32 Index = Frozen->GetResourceIndex();
        const FShaderMapResourceCode* Code = Map->GetResourceCode();
        const int32 OriginalFrequency = int32(Record->GetNumberField(TEXT("frequency")));
        if (!OutputHashes.IsValidIndex(ShaderNumber) || !Code->ShaderHashes.IsValidIndex(Index) ||
            !Code->ShaderEntries.IsValidIndex(Index) || Code->ShaderHashes[Index] != OutputHashes[ShaderNumber] ||
            int32(Code->ShaderEntries[Index].Frequency) != OriginalFrequency)
        { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: frozen shader resource mismatch")); return; }
        ++ShaderNumber;
        Types.Add(Type);
    }
    bool bRhiPassed = true;
    ENQUEUE_RENDER_COMMAND(CheckLegacySlateRHI)([Map, Types, &bRhiPassed](FRHICommandListImmediate&)
    {
        for (FShaderType* Type : Types)
        {
            const TShaderRef<FShader> Shader = Map->GetShader(Type, 0);
            if (!Shader.IsValid() || (Shader->GetFrequency() == SF_Vertex ? Shader.GetVertexShader() == nullptr : Shader.GetPixelShader() == nullptr))
            { bRhiPassed = false; return; }
        }
    });
    FlushRenderingCommands();
    if (!bRhiPassed) { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: RHI shader creation failed")); return; }
    UE_LOG(LogTemp, Display, TEXT("Legacy material probe: detached map and seven RHI shaders constructed"));
    if (FParse::Param(FCommandLine::Get(), TEXT("FortniteLegacyMaterialDraw")))
    {
        if (!bHaveVSBindings || !bHavePSBindings || !DrawLegacySlateQuad(Map, DrawBindings, Error))
        { UE_LOG(LogTemp, Error, TEXT("Legacy material probe draw: %s"), *Error); return; }
    }
#endif
}
