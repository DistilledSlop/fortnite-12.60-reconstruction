#include "CoreMinimal.h"

#if PLATFORM_LINUX && !UE_SERVER
#include "Misc/Crc.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "Materials/Material.h"
#include "RenderingThread.h"
#include "Misc/FileHelper.h"
#include "Misc/SecureHash.h"
#include "LegacySlateMaterial.h"
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
    FLegacySlateShaderInput& Input, FString& Error)
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
    if (FParse::Value(FCommandLine::Get(), TEXT("FortniteLegacyMaterialArtifactDir="), ArtifactDirectory))
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

static bool CheckLegacyUniforms(const FUniformExpressionSet& Uniforms, const TSharedPtr<FJsonObject>& Root, FString& Error)
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
    UE_LOG(LogTemp, Display, TEXT("Legacy material probe: eight live uniform cases passed"));
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
#endif

void RunLegacyMaterialShaderMapProbe()
{
#if PLATFORM_LINUX && !UE_SERVER
    FString FixturePath;
    if (!FParse::Value(FCommandLine::Get(), TEXT("FortniteLegacyMaterialFixture="), FixturePath)) return;
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
    TRefCountPtr<FMaterialShaderMap> Map = FMaterialShaderMap::CreateLegacySlateMap(IR,
        [&Shaders, &Root](const FUniformExpressionSet& Uniforms, const FShaderParametersMetadata&, TArray<FLegacySlateShaderInput>& Outputs, FString& OutError)
        {
            if (!CheckLegacyUniforms(Uniforms, Root, OutError)) return false;
            const FRHIUniformBufferLayout& Layout = Uniforms.GetUniformBufferLayout();
            for (const auto& Value : Shaders)
            {
                const auto Record = Value->AsObject();
                if (!Record) { OutError = TEXT("invalid shader record"); return false; }
                FLegacySlateShaderInput& Input = Outputs.AddDefaulted_GetRef();
                if (!PrepareLegacyOutput(Record, Layout, Input, OutError)) return false;
            }
            return true;
        }, Error);
    if (!Map.IsValid())
    { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: %s"), *Error); return; }
    TArray<FShaderType*> Types;
    for (const auto& Shader : Shaders)
    {
        const auto Record = Shader->AsObject()->GetObjectField(TEXT("record"));
        FShaderType* Type = FindShaderTypeByName(FHashedName(*Record->GetObjectField(TEXT("type"))->GetStringField(TEXT("name"))));
        if (!Type || !Map->GetShader(Type, 0).IsValid())
        { UE_LOG(LogTemp, Error, TEXT("Legacy material probe: frozen shader lookup failed")); return; }
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
#endif
}
