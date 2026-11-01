#include "CoreMinimal.h"

#if PLATFORM_LINUX && !UE_SERVER
#include "DynamicRHI.h"
#include "Misc/Crc.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Materials/Material.h"
#include "RenderingThread.h"
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
    FLegacySlateShaderInput& Input, FString& Error, bool bWriteArtifacts = true)
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
    bool bPacked = false, bMaterial = false, bSampler = false, bTexture = false;
    for (const TSharedPtr<FJsonValue>& Value : DescriptorValues)
    {
        const TSharedPtr<FJsonObject> D = Value->AsObject();
        if (!D) { Error = TEXT("invalid descriptor"); return false; }
        const uint32 Id = uint32(D->GetNumberField(TEXT("variable_id")));
        const FLegacySpirvDescriptor* S = Decorations.Find(Id);
        const FString Name = D->GetStringField(TEXT("name"));
        if (!S || S->Set != uint32(D->GetNumberField(TEXT("set"))) || S->Binding != uint32(D->GetNumberField(TEXT("binding"))) || S->Set != 0 || S->Binding > UINT16_MAX || D->GetArrayField(TEXT("resource_table_entries")).Num())
        { Error = TEXT("descriptor receipt mismatch"); return false; }
        const auto Register = D->GetObjectField(TEXT("d3d_register"));
        const TCHAR* ExpectedKind = Name.StartsWith(TEXT("cb")) ? TEXT("cbv") : Name == TEXT("s0") ? TEXT("sampler") : Name == TEXT("t0") ? TEXT("srv") : TEXT("");
        const int32 ExpectedIndex = Name == TEXT("cb1_1") ? 1 : 0;
        if (!Register || Register->GetStringField(TEXT("kind")) != ExpectedKind ||
            int32(Register->GetNumberField(TEXT("space"))) != 0 || int32(Register->GetNumberField(TEXT("index"))) != ExpectedIndex)
        { Error = TEXT("D3D register receipt mismatch"); return false; }
        const int32 DescriptorType = int32(D->GetNumberField(TEXT("vulkan_descriptor_type")));
        if (Name == TEXT("cb0_0") && !bPacked && DescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        {
            const int32 Span = int32(D->GetNumberField(TEXT("block_span_bytes")));
            if (Span != (bVertex ? 80 : 64)) { Error = TEXT("unexpected packed buffer span"); return false; }
            auto& P = Header.PackedUBs.AddDefaulted_GetRef();
            P.SizeInBytes = Span; P.OriginalBindingIndex = S->Binding;
            P.PackedTypeIndex = CrossCompiler::EPackedTypeIndex::HighP;
            P.SPIRVDescriptorSetOffset = S->SetOffset; P.SPIRVBindingIndexOffset = S->BindingOffset;
            bPacked = true;
        }
        else if (Name == TEXT("cb1_1") && !bVertex && !bMaterial && DescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        {
            if (int32(D->GetNumberField(TEXT("block_span_bytes"))) != 320 || Layout.GetHash() == 0)
            { Error = TEXT("unexpected Material descriptor"); return false; }
            auto& U = Header.UniformBuffers.AddDefaulted_GetRef();
            U.LayoutHash = Layout.GetHash(); U.ConstantDataOriginalBindingIndex = S->Binding; U.bOnlyHasResources = 0;
            Header.UniformBufferSpirvInfos.Emplace(S->SetOffset, S->BindingOffset);
            Input.Output.ParameterMap.AddParameterAllocation(TEXT("Material"), 0, FVulkanShaderHeader::UniformBuffer, 1, EShaderParameterType::UniformBuffer);
            bMaterial = true;
        }
        else if ((Name == TEXT("s0") && !bSampler && DescriptorType == VK_DESCRIPTOR_TYPE_SAMPLER) ||
                 (Name == TEXT("t0") && !bTexture && DescriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE))
        {
            if (bVertex) { Error = TEXT("unexpected vertex descriptor"); return false; }
            const bool bIsSampler = Name == TEXT("s0");
            const uint16 Index = Header.Globals.Num();
            auto& G = Header.Globals.AddDefaulted_GetRef();
            G.OriginalBindingIndex = S->Binding; G.CombinedSamplerStateAliasIndex = UINT16_MAX;
            G.TypeIndex = Header.GlobalDescriptorTypes.Add(static_cast<VkDescriptorType>(DescriptorType));
            Header.GlobalSpirvInfos.Emplace(S->SetOffset, S->BindingOffset);
            Input.Output.ParameterMap.AddParameterAllocation(bIsSampler ? TEXT("ElementTextureSampler") : TEXT("ElementTexture"),
                FVulkanShaderHeader::Global, Index, 1, bIsSampler ? EShaderParameterType::Sampler : EShaderParameterType::SRV);
            if (bIsSampler) bSampler = true; else bTexture = true;
        }
        else { Error = TEXT("unsupported descriptor"); return false; }
    }
    if (!bPacked || bMaterial == bVertex || bSampler != bTexture)
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
    Input.Output.NumTextureSamplers = bSampler ? 1 : 0;
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
    virtual const FMaterial& GetMaterialWithFallback(ERHIFeatureLevel::Type Feature, const FMaterialRenderProxy*& Fallback) const override
    { return Base->GetMaterialWithFallback(Feature, Fallback); }
    virtual bool GetVectorValue(const FHashedMaterialParameterInfo& Info, FLinearColor* Value, const FMaterialRenderContext&) const override
    { for (const auto& O : Overrides) if (O.bVector && O.Parameter == Info) { *Value = O.Value; return true; } return false; }
    virtual bool GetScalarValue(const FHashedMaterialParameterInfo& Info, float* Value, const FMaterialRenderContext&) const override
    { for (const auto& O : Overrides) if (!O.bVector && O.Parameter == Info) { *Value = O.Value.R; return true; } return false; }
    virtual bool GetTextureValue(const FHashedMaterialParameterInfo&, const UTexture**, const FMaterialRenderContext&) const override { return false; }
    virtual bool GetTextureValue(const FHashedMaterialParameterInfo&, const URuntimeVirtualTexture**, const FMaterialRenderContext&) const override { return false; }
};

static bool CheckLegacyUniforms(const FUniformExpressionSet& Uniforms, const TSharedPtr<FJsonObject>& Root, const TCHAR* Stage, FString& Error)
{
    TArray<FLegacyUniformCase> Cases;
    const auto& Values = Root->GetArrayField(TEXT("uniform_cases"));
    if (Values.Num() != 8) { Error = TEXT("invalid uniform cases"); return false; }
    for (const auto& Value : Values)
    {
        const auto Case = Value->AsObject();
        auto& C = Cases.AddDefaulted_GetRef();
        C.Name = Case->GetStringField(TEXT("name"));
        const auto& Expected = Case->GetArrayField(TEXT("numeric_values"));
        if (Expected.Num() != 80) { Error = TEXT("invalid numeric reference"); return false; }
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
    UMaterial* DefaultMaterial = UMaterial::GetDefaultMaterial(MD_Surface);
    if (!DefaultMaterial) { Error = TEXT("default material unavailable"); return false; }
    const FMaterialRenderProxy* Base = DefaultMaterial->GetRenderProxy();
    bool bPassed = true;
    FString Failure;
    ENQUEUE_RENDER_COMMAND(CheckLegacyUniformExpressions)([&Uniforms, &Cases, Base, &bPassed, &Failure](FRHICommandListImmediate&)
    {
        FLegacyProbeProxy Proxy;
        Proxy.Base = Base;
        const FMaterialRenderProxy* Fallback = nullptr;
        const FMaterial& Material = Base->GetMaterialWithFallback(ERHIFeatureLevel::SM5, Fallback);
        FMaterialRenderContext Context(&Proxy, Material, nullptr);
        FUniformExpressionCache Cache;
        for (const auto& C : Cases)
        {
            Proxy.Overrides = C.Overrides;
            alignas(16) uint8 Buffer[336] = {};
            Uniforms.FillUniformBuffer(Context, Cache, Buffer, sizeof(Buffer));
            const float* Actual = reinterpret_cast<const float*>(Buffer);
            for (int32 I = 0; I < 78; ++I)
            {
                const float Expected = C.Expected[I];
                if (!FMath::IsFinite(Actual[I]) || FMath::Abs(Actual[I] - Expected) > 1.e-5f + 1.e-5f * FMath::Abs(Expected))
                { bPassed = false; Failure = FString::Printf(TEXT("uniform case %s slot %d: actual %g expected %g"), *C.Name, I, Actual[I], Expected); return; }
            }
        }
    });
    FlushRenderingCommands();
    if (!bPassed) { Error = Failure; return false; }
    UE_LOG(LogTemp, Display, TEXT("Legacy material probe: eight %s live uniform cases passed"), Stage);
    return true;
}

struct FLegacyDrawBindings
{
    uint16 MatrixBuffer = 0, MatrixOffset = 0, GammaBuffer = 0, GammaOffset = 0;
    uint16 ParamsBuffer = 0, ParamsOffset = 0, MaterialBuffer = 0;
};

static bool DrawLegacySlateQuad(const TRefCountPtr<FMaterialShaderMap>& Map, const FLegacyDrawBindings& Bindings, FString& Error, UMaterial* Owner = nullptr, const FMaterial* ExpectedOwnerResource = nullptr)
{
    FShaderType* VertexType = FindShaderTypeByName(FHashedName(TEXT("TSlateMaterialShaderVSfalse")));
    FShaderType* PixelType = FindShaderTypeByName(FHashedName(TEXT("TSlateMaterialShaderPSDefault")));
    UMaterial* DefaultMaterial = Owner ? Owner : UMaterial::GetDefaultMaterial(MD_Surface);
    if (!VertexType || !PixelType || !DefaultMaterial) { Error = TEXT("draw shader or material unavailable"); return false; }
    const FMaterialRenderProxy* Base = DefaultMaterial->GetRenderProxy();
    bool bPassed = true;
    FString Failure;
    TArray<FColor> Images[2];
    int32 Different = 0;
    ENQUEUE_RENDER_COMMAND(DrawOriginalLockerSlate)([Map, VertexType, PixelType, Base, Bindings, ExpectedOwnerResource, &bPassed, &Failure, &Images, &Different](FRHICommandListImmediate& RHICmdList)
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
        FLegacyProbeProxy Proxy; Proxy.Base = Base;
        const FMaterialRenderProxy* Fallback = nullptr;
        const FMaterial& Material = Base->GetMaterialWithFallback(ERHIFeatureLevel::SM5, Fallback);
        if (ExpectedOwnerResource && &Material != ExpectedOwnerResource) { bPassed=false; Failure=TEXT("owner render proxy selected a fallback resource"); return; }
        FMaterialRenderContext Context(&Proxy, Material, nullptr);
        FUniformExpressionCache Cache;
        for (int32 Pass = 0; Pass < 2; ++Pass)
        {
            Proxy.Overrides.Reset();
            for (const TCHAR* Name : {TEXT("RBGA1")})
            {
                auto& O = Proxy.Overrides.AddDefaulted_GetRef();
                O.Parameter = FHashedMaterialParameterInfo(Name);
                O.bVector = true;
                O.Value = Pass == 0 ? FLinearColor(1, 0, 0, 1) : FLinearColor(0, 1, 0, 1);
            }
            alignas(16) uint8 Contents[336] = {};
            const FUniformExpressionSet& Uniforms = Map->GetUniformExpressionSet();
            Uniforms.FillUniformBuffer(Context, Cache, Contents, sizeof(Contents));
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
        if (!Different) { bPassed = false; Failure = TEXT("override draws produced identical pixels"); }
        UE_LOG(LogTemp, Display, TEXT("Legacy material probe: original Slate quad readback changed %d pixels"), Different);
    });
    FlushRenderingCommands();
    FString ArtifactDirectory;
    if (Images[0].Num() == 4096 && Images[1].Num() == 4096 && FParse::Value(FCommandLine::Get(), TEXT("FortniteLegacyMaterialArtifactDir="), ArtifactDirectory))
    {
        FSHAHash Hashes[2];
        for (int32 I = 0; I < 2; ++I)
        {
            const FString Path = FPaths::Combine(ArtifactDirectory, I == 0 ? TEXT("LockerQuad-Red.rgba") : TEXT("LockerQuad-Green.rgba"));
            if (FPaths::FileExists(Path)) { Error = TEXT("draw artifact already exists"); return false; }
            TArray<uint8> Bytes;
            for (const FColor& Pixel : Images[I]) { Bytes.Add(Pixel.R); Bytes.Add(Pixel.G); Bytes.Add(Pixel.B); Bytes.Add(Pixel.A); }
            FSHA1::HashBuffer(Bytes.GetData(), Bytes.Num(), Hashes[I].Hash);
            if (!FFileHelper::SaveArrayToFile(Bytes, *Path)) { Error = TEXT("cannot write draw artifact"); return false; }
        }
        const FString MetadataPath = FPaths::Combine(ArtifactDirectory, TEXT("LockerQuad.json"));
        const FString Metadata = FString::Printf(TEXT("{\n  \"width\": 64, \"height\": 64, \"format\": \"RGBA8\",\n  \"red_sha1\": \"%s\", \"green_sha1\": \"%s\",\n  \"changed_pixels\": %d\n}\n"), *Hashes[0].ToString(), *Hashes[1].ToString(), Different);
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
    if (!Expected || int32(Expected->GetNumberField(TEXT("constant_bytes"))) != 336 ||
        Expected->GetArrayField(TEXT("resource_offsets")).Num() != 2 || Expected->GetArrayField(TEXT("resource_types")).Num() != 2 ||
        int32(Expected->GetArrayField(TEXT("resource_offsets"))[0]->AsNumber()) != 320 ||
        int32(Expected->GetArrayField(TEXT("resource_offsets"))[1]->AsNumber()) != 328 ||
        int32(Expected->GetArrayField(TEXT("resource_types"))[0]->AsNumber()) != 8 ||
        int32(Expected->GetArrayField(TEXT("resource_types"))[1]->AsNumber()) != 8)
    { Error = TEXT("unexpected original layout"); return false; }
    IR.UniformBufferSize = 336;
    IR.Resources.Add({320, UBMT_SAMPLER});
    IR.Resources.Add({328, UBMT_SAMPLER});
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
        if (Roots.Num() != (Which == 0 ? 12 : 30)) { Error = TEXT("unexpected root count"); return false; }
        for (const auto& RootValue : Roots)
        {
            int32 Index = int32(RootValue->AsNumber());
            if (!IR.Expressions.IsValidIndex(Index)) { Error = TEXT("invalid root"); return false; }
            (Which == 0 ? IR.VectorRoots : IR.ScalarRoots).Add(Index);
        }
    }
    return true;
}
// Registered before asset loading; this callback performs CPU-only conversion.
static FString GLegacyIngressBundleJson;
static const TCHAR* GLegacyLockerPath = TEXT("/Game/UI/Foundation/Materials/Athena_LockerTabMaster.Athena_LockerTabMaster");
struct FLockerProvenance final : FLegacyMaterialAssetProvenance
{
    FLegacyOriginalMaterial Decoded;
    FString BundleJson;
    int32 ArchiveUE4Version = 0, ArchiveLicenseeVersion = 0;
};

static bool SameOriginalExpressions(const FLegacyMaterialIR& A, const FLegacyMaterialIR& B)
{
    if (A.OriginalMapHash != B.OriginalMapHash || A.QualityLevel != B.QualityLevel || A.FeatureLevel != B.FeatureLevel ||
        A.VectorRoots != B.VectorRoots || A.ScalarRoots != B.ScalarRoots || A.Expressions.Num() != B.Expressions.Num()) return false;
    for (int32 I=0; I<A.Expressions.Num(); ++I)
    {
        const auto& X=A.Expressions[I]; const auto& Y=B.Expressions[I];
        if (X.Kind!=Y.Kind || X.A!=Y.A || X.B!=Y.B || X.C!=Y.C || X.Value!=Y.Value ||
            X.Parameter!=Y.Parameter || X.ValueType!=Y.ValueType || X.Operation!=Y.Operation || X.Components!=Y.Components ||
            FMemory::Memcmp(X.Swizzle,Y.Swizzle,sizeof(X.Swizzle))) return false;
    }
    return true;
}

static ELegacyMaterialAssetResult ProvideLockerOriginal(const FLegacyMaterialAssetContext& Context,
    TRefCountPtr<FMaterialShaderMap>& Map, TSharedPtr<const FLegacyMaterialAssetProvenance, ESPMode::ThreadSafe>& Provenance, FString& Error)
{
    if (Context.OwnerPath != GLegacyLockerPath) return ELegacyMaterialAssetResult::NotApplicable;
    auto Fail = [&Error](const TCHAR* Why) { Error=Why; return ELegacyMaterialAssetResult::Failed; };
    TSharedPtr<FJsonObject> Bundle;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(GLegacyIngressBundleJson),Bundle) || !Bundle)
        return Fail(TEXT("cannot parse pinned Locker bundle"));
    if (Context.ResourceEnd-Context.ResourceBegin != 12513 ||
        Context.Archive.Tell()!=Context.ResourceBegin+8) return Fail(TEXT("Locker resource bounds mismatch"));
    TSharedRef<FLockerProvenance, ESPMode::ThreadSafe> P=MakeShared<FLockerProvenance, ESPMode::ThreadSafe>();
    P->ArchiveUE4Version=Context.Archive.UE4Ver();
    P->ArchiveLicenseeVersion=Context.Archive.LicenseeUE4Ver();
    UE_LOG(LogTemp, Display, TEXT("Legacy material ingress: Locker archive UE4=%d licensee=%d"), P->ArchiveUE4Version, P->ArchiveLicenseeVersion);
    P->Decoded.Bytes.SetNumUninitialized(int32(Context.ResourceEnd-Context.ResourceBegin));
    P->Decoded.Names.Append(Context.Names.GetData(), Context.Names.Num());
    const auto Framing=Bundle->GetObjectField(TEXT("framing"));
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
    if (!OriginalHash.ToString().Equals(TEXT("bc2d2f509b6909a4820f65188b85485fce989a2a"),ESearchCase::IgnoreCase))
        return Fail(TEXT("Locker source digest mismatch"));
    if (!DecodeLegacyOriginalMaterial(P->Decoded,Error)) return ELegacyMaterialAssetResult::Failed;
    if (P->Decoded.MaterialPath != GLegacyLockerPath || P->Decoded.Records.Num()!=19) return Fail(TEXT("Locker decoded identity mismatch"));
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
        [&Shaders](const FUniformExpressionSet& Uniforms, const FShaderParametersMetadata&, TArray<FLegacySlateShaderInput>& Outputs, FString& OutError)
        {
            for (const auto& Value : Shaders)
            {
                const auto Shader=Value->AsObject();
                if (!Shader || !PrepareLegacyOutput(Shader,Uniforms.GetUniformBufferLayout(),Outputs.AddDefaulted_GetRef(),OutError,false)) return false;
            }
            return true;
        },Error);
    if (!Map.IsValid()) return ELegacyMaterialAssetResult::Failed;
    P->OriginalResource=P->Decoded.Bytes;
    FSHA1::HashBuffer(P->OriginalResource.GetData(),P->OriginalResource.Num(),P->ResourceHash.Hash);
    P->SourceIdentity=GLegacyLockerPath; P->OriginalShaderRecordCount=P->Decoded.Records.Num(); P->BundleJson=GLegacyIngressBundleJson;
    Provenance=P; return ELegacyMaterialAssetResult::Converted;
}
#endif

void RegisterLegacyOriginalMaterialProvider()
{
#if PLATFORM_LINUX && !UE_SERVER
    FString Path;
    if (!FParse::Value(FCommandLine::Get(),TEXT("FortniteLegacyMaterialIngress="),Path)) return;
    // An invalid receipt still installs the exact-owner provider, which fails closed.
    auto RejectConfig=[](const TCHAR* Why) { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: %s"),Why); RegisterLegacyMaterialAssetProvider(&ProvideLockerOriginal); };
    TArray<uint8> Bytes; FSHAHash Hash;
    if (!FFileHelper::LoadFileToArray(Bytes,*Path) || Bytes.Num()>32*1024*1024)
    { RejectConfig(TEXT("cannot read bundle")); return; }
    FSHA1::HashBuffer(Bytes.GetData(),Bytes.Num(),Hash.Hash);
    if (!Hash.ToString().Equals(TEXT("0f750204f6008e6e3219d0b002d8409e28c22426"),ESearchCase::IgnoreCase))
    { RejectConfig(TEXT("bundle digest mismatch")); return; }
    FFileHelper::BufferToString(GLegacyIngressBundleJson,Bytes.GetData(),Bytes.Num());
    TSharedPtr<FJsonObject> Bundle;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(GLegacyIngressBundleJson),Bundle) || !Bundle ||
        Bundle->GetStringField(TEXT("schema"))!=TEXT("fortnite.legacy-material-ingress.v1"))
    { GLegacyIngressBundleJson.Reset(); RejectConfig(TEXT("invalid bundle")); return; }
    FModuleManager::LoadModuleChecked<IModuleInterface>(TEXT("SlateRHIRenderer"));
    RegisterLegacyMaterialAssetProvider(&ProvideLockerOriginal);
    UE_LOG(LogTemp,Display,TEXT("Legacy material ingress: registered pinned Locker provider"));
#endif
}

void ProbeLegacyOriginalMaterial()
{
#if PLATFORM_LINUX && !UE_SERVER
    if (!FParse::Param(FCommandLine::Get(),TEXT("FortniteProbeLegacyOriginalMaterial"))) return;
    TSharedPtr<FJsonObject> Bundle;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(GLegacyIngressBundleJson),Bundle) || !Bundle)
    { UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: probe has no valid bundle")); FPlatformMisc::RequestExitWithStatus(false,1); return; }
    UMaterial* Material=LoadObject<UMaterial>(nullptr,GLegacyLockerPath);
    FMaterialResource* Resource=Material ? Material->GetMaterialResource(GMaxRHIFeatureLevel) : nullptr;
    FMaterialShaderMap* Map=Resource ? Resource->GetGameThreadShaderMap() : nullptr;
    const auto Provenance=Map ? Map->GetLegacyMaterialProvenance() : nullptr;
    const bool bOriginal=Provenance.IsValid() && Provenance->OriginalShaderRecordCount==19 &&
        Provenance->SourceIdentity==GLegacyLockerPath &&
        Provenance->ResourceHash.ToString().Equals(TEXT("bc2d2f509b6909a4820f65188b85485fce989a2a"),ESearchCase::IgnoreCase);
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
    const auto* Original=static_cast<const FLockerProvenance*>(Provenance.Get());
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
    FLegacyDrawBindings Bindings; bool bVS=false, bPS=false, bAllShaders=true; int32 CheckedShaders=0; FString Error;
    const auto Fixture=Bundle->GetObjectField(TEXT("runtime_fixture"));
    for (const auto& Value : Fixture->GetArrayField(TEXT("shaders")))
    {
        const auto Shader=Value->AsObject(); FLegacySlateShaderInput Input;
        if (!PrepareLegacyOutput(Shader,Map->GetUniformExpressionSet().GetUniformBufferLayout(),Input,Error)) { bAllShaders=false; break; }
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
    }
    if (!bAllShaders || CheckedShaders!=7 || !bVS || !bPS || !DrawLegacySlateQuad(Map,Bindings,Error,Material,Resource))
    {
        UE_LOG(LogTemp,Error,TEXT("Legacy material ingress: attached owner draw failed: %s"),*Error);
        FPlatformMisc::RequestExitWithStatus(false,1); return;
    }
    UE_LOG(LogTemp,Display,TEXT("Legacy material ingress: original Locker owner attached complete map with 19 original records on game and render threads and drew its Slate quad"));
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
