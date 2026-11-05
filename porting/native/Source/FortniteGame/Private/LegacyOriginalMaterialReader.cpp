#include "LegacyOriginalMaterialReader.h"
#if PLATFORM_LINUX && !UE_SERVER

namespace
{
class FReader
{
    FLegacyOriginalMaterial& O;
    FString& Error;
    int32 P = 0, Limit, Nodes = 0;
    int32 Counts[7] = {};
    bool bOk = true, bPaper2D = false, bLandscape = false;
    struct FPendingTextureSize { int32 Node; FLegacyMaterialTextureParameter Texture; };
    TArray<FPendingTextureSize> PendingTextureSizes;
public:
    FReader(FLegacyOriginalMaterial& In, FString& E) : O(In), Error(E), Limit(In.Bytes.Num()) {}
    bool Fail(const TCHAR* Why) { if (bOk) Error = FString::Printf(TEXT("%s at resource offset %d"), Why, P); bOk = false; return false; }
    template<typename T> T Read()
    {
        T V{};
        if (!bOk || P < 0 || sizeof(T) > uint32(Limit - P)) { Fail(TEXT("truncated field")); return V; }
        FMemory::Memcpy(&V, O.Bytes.GetData() + P, sizeof(T)); P += sizeof(T); return V;
    }
    int32 Count() { int32 N = Read<int32>(); if (N < 0 || N > 4096) Fail(TEXT("invalid array count")); return bOk ? N : 0; }
    FName Name()
    {
        int32 I = Read<int32>(), N = Read<int32>();
        if (!O.Names.IsValidIndex(I) || N < 0) { Fail(TEXT("invalid local name")); return NAME_None; }
        return FName(O.Names[I], N);
    }
    FString String()
    {
        int32 N = Read<int32>();
        if (N < 1 || N > 4096 || N > Limit - P) { Fail(TEXT("invalid string length")); return {}; }
        const ANSICHAR* S = reinterpret_cast<const ANSICHAR*>(O.Bytes.GetData() + P);
        int32 Length = 0; while (Length < N && S[Length]) ++Length;
        if (Length != N-1) { Fail(TEXT("invalid string terminator")); return {}; }
        FString R(UTF8_TO_TCHAR(S)); P += N; return R;
    }
    FSHAHash Hash() { FSHAHash H; for (uint8& B : H.Hash) B = Read<uint8>(); return H; }
    FLegacyOriginalUniformBinding Uniform() { FLegacyOriginalUniformBinding B; B.BaseIndex=Read<uint16>(); uint32 Bound=Read<uint32>(); if (Bound>1) Fail(TEXT("invalid uniform binding")); B.bBound=Bound!=0; return B; }
    FLegacyOriginalParameter Param(bool Resource = false) { FLegacyOriginalParameter B; B.BaseIndex=Read<uint16>(); B.Size=Read<uint16>(); if (!Resource) B.BufferIndex=Read<uint16>(); return B; }
    FLegacyOriginalPair Pair() { FLegacyOriginalPair V; V.First=Read<uint16>(); V.Second=Read<uint16>(); return V; }
    void ParameterInfo(FLegacyMaterialExpression& E)
    {
        FName N = Name(); uint8 A = Read<uint8>(); int32 I = Read<int32>();
        if (A > 2 || I < -1) Fail(TEXT("invalid parameter identity"));
        E.Parameter = FMaterialParameterInfo(N, static_cast<EMaterialParameterAssociation>(A), I);
    }
    int32 Expression(int32 Depth, bool bTextureRoot = false, bool bDirectVectorRoot = false, bool bVirtualRoot = false)
    {
        if (Depth >= 64 || ++Nodes > 65536) { Fail(TEXT("expression complexity")); return INDEX_NONE; }
        FString T = Name().ToString(); const FString Prefix(TEXT("FMaterialUniformExpression"));
        if (!T.RemoveFromStart(Prefix)) { Fail(TEXT("invalid expression tag")); return INDEX_NONE; }
        FLegacyMaterialExpression E;
        if (bTextureRoot && T != TEXT("TextureParameter") && T != TEXT("Texture")) Fail(TEXT("invalid texture root"));
        if (T == TEXT("Constant")) { E.Kind = ELegacyMaterialExpression::Constant; E.Value.R=Read<float>(); E.Value.G=Read<float>(); E.Value.B=Read<float>(); E.Value.A=Read<float>(); E.ValueType=Read<uint8>(); }
        else if (T == TEXT("ScalarParameter") || T == TEXT("VectorParameter"))
        {
            E.Kind = T == TEXT("ScalarParameter") ? ELegacyMaterialExpression::ScalarParameter : ELegacyMaterialExpression::VectorParameter;
            ParameterInfo(E); E.Value.R=Read<float>();
            if (T == TEXT("VectorParameter")) { E.Value.G=Read<float>(); E.Value.B=Read<float>(); E.Value.A=Read<float>(); }
        }
        else if (T == TEXT("RuntimeVirtualTextureUniform"))
        {
            // The original serializer always writes ParameterInfo, even for
            // a non-parameter expression. Preserve all fields verbatim.
            const uint32 Parameter = Read<uint32>();
            if (Parameter > 1) Fail(TEXT("invalid runtime virtual texture parameter flag"));
            E.Kind=ELegacyMaterialExpression::RuntimeVirtualTextureUniform; E.ValueType=MCT_Float4;
            E.bRuntimeVirtualTextureParameter=Parameter != 0;
            ParameterInfo(E);
            E.RuntimeVirtualTextureIndex=Read<int32>();
            E.RuntimeVirtualTextureVectorIndex=Read<int32>();
        }
        else if (T == TEXT("TextureParameter") || T == TEXT("Texture"))
        {
            if (T == TEXT("TextureParameter"))
            {
                ParameterInfo(E);
                if (E.Parameter.Name.IsNone()) Fail(TEXT("named texture parameter has no name"));
            }
            // For the original plain Texture expression the canonical default
            // FMaterialParameterInfo is NAME_None, GlobalParameter, INDEX_NONE.
            int32 Index=Read<int32>(); int16 Layer=Read<int16>(), Page=Read<int16>(); int32 Sampler=Read<int32>(); uint32 Virtual=Read<uint32>();
            if (Index < -1 || Layer < -1 || Page < -1 || Sampler < 0 || Sampler > 2 || Virtual > 1) Fail(TEXT("invalid texture expression"));
            if (!bTextureRoot || Depth != 0 || Index < 0 ||
                (bVirtualRoot ? (Layer < 0 || Page < 0 || Virtual != 1) : (Layer != -1 || Page != -1 || Virtual != 0)))
                Fail(TEXT("unsupported texture expression"));
            FLegacyMaterialTextureParameter Texture;
            Texture.Parameter=E.Parameter; Texture.TextureIndex=Index; Texture.TextureLayerIndex=Layer;
            Texture.PageTableLayerIndex=Page; Texture.SamplerSource=uint8(Sampler); Texture.bVirtualTexture=Virtual != 0;
            if (bVirtualRoot) O.IR.VirtualTextureParameters.Add(MoveTemp(Texture));
            else O.IR.Texture2DParameters.Add(MoveTemp(Texture));
            return INDEX_NONE;
        }
        else if (T == TEXT("TextureProperty"))
        {
            if (Depth != 0 || !bDirectVectorRoot || bTextureRoot)
            { Fail(TEXT("TextureProperty requires a direct vector root")); return INDEX_NONE; }
            if (++Nodes > 65536 || Name() != FName(TEXT("FMaterialUniformExpressionTexture")))
            { Fail(TEXT("direct texture size requires a regular Texture child")); return INDEX_NONE; }
            FLegacyMaterialTextureParameter Texture;
            Texture.TextureIndex=Read<int32>(); Texture.TextureLayerIndex=Read<int16>(); Texture.PageTableLayerIndex=Read<int16>();
            const int32 Sampler=Read<int32>(); const uint32 Virtual=Read<uint32>(); const uint8 Property=Read<uint8>();
            if (Property != 0 || Texture.TextureIndex < 0 || Texture.TextureLayerIndex != -1 ||
                Texture.PageTableLayerIndex != -1 || Sampler < 0 || Sampler > 2 || Virtual != 0)
            { Fail(TEXT("unsupported direct texture-size payload")); return INDEX_NONE; }
            Texture.SamplerSource=uint8(Sampler); Texture.bVirtualTexture=false;
            E.Kind=ELegacyMaterialExpression::TextureSize2D; E.ValueType=MCT_Float4;
            PendingTextureSizes.Add({O.IR.Expressions.Num(),Texture});
        }
        else if (T == TEXT("Max") || T == TEXT("Saturate") || T == TEXT("Abs"))
        {
            E.A=Expression(Depth+1);
            if (T == TEXT("Max")) { E.B=Expression(Depth+1); E.Kind=ELegacyMaterialExpression::Max; }
            else E.Kind=T == TEXT("Abs") ? ELegacyMaterialExpression::Abs : ELegacyMaterialExpression::Saturate;
        }
        else if (T == TEXT("ComponentSwizzle") || T == TEXT("FoldedMath") || T == TEXT("AppendVector") || T == TEXT("TrigMath"))
        {
            E.A=Expression(Depth+1); if (T != TEXT("ComponentSwizzle")) E.B=Expression(Depth+1);
            if (T == TEXT("ComponentSwizzle")) { E.Kind=ELegacyMaterialExpression::ComponentSwizzle; for (int8& C : E.Swizzle) { C=Read<int8>(); if (C < -1 || C > 3) Fail(TEXT("invalid swizzle")); } E.Components=Read<uint8>(); if (E.Components < 1 || E.Components > 4) Fail(TEXT("invalid swizzle size")); }
            else if (T == TEXT("FoldedMath")) { E.Kind=ELegacyMaterialExpression::FoldedMath; E.Operation=Read<uint8>(); E.ValueType=Read<uint32>(); if (E.Operation > 5) Fail(TEXT("invalid math operation")); }
            else if (T == TEXT("AppendVector")) { E.Kind=ELegacyMaterialExpression::AppendVector; uint32 C=Read<uint32>(); if (C < 1 || C > 4) Fail(TEXT("invalid append size")); E.Components=uint8(C); }
            else { E.Kind=ELegacyMaterialExpression::TrigMath; E.Operation=Read<uint8>(); if (E.Operation > 6) Fail(TEXT("invalid trig operation")); }
        }
        else { Fail(TEXT("unsupported expression")); return INDEX_NONE; }
        return O.IR.Expressions.Add(MoveTemp(E));
    }
    void Record(bool Mesh, FName VF, FName ExpectedPipeline=NAME_None)
    {
        FLegacyOriginalRecord R; R.Begin=P; FName Type=Name(); R.Type=Type.ToString(); int64 End=Read<int64>();
        static const TSet<FString> Allowed = {
            TEXT("TSlateMaterialShaderVStrue"), TEXT("TSlateMaterialShaderVSfalse"),
            TEXT("TSlateMaterialShaderPSGrayscaleFont"), TEXT("TSlateMaterialShaderPSDefault"), TEXT("TSlateMaterialShaderPSCustom"), TEXT("TSlateMaterialShaderPSColorFont"), TEXT("TSlateMaterialShaderPSBorder"),
            TEXT("TTranslucencyShadowDepthVS<TranslucencyShadowDepth_Standard>"), TEXT("TTranslucencyShadowDepthVS<TranslucencyShadowDepth_PerspectiveCorrect>"),
            TEXT("TTranslucencyShadowDepthPS<TranslucencyShadowDepth_Standard>"), TEXT("TTranslucencyShadowDepthPS<TranslucencyShadowDepth_PerspectiveCorrect>"),
            TEXT("TBasePassVSFSimpleNoLightmapLightingPolicy"), TEXT("TBasePassVSFNoLightMapPolicy"), TEXT("FConvertToUniformMeshVS"), TEXT("FConvertToUniformMeshGS"),
            TEXT("TBasePassPSFSimpleNoLightmapLightingPolicy"), TEXT("TBasePassPSFSimpleNoLightmapLightingPolicySkylight"), TEXT("TBasePassPSFNoLightMapPolicy"), TEXT("TBasePassPSFNoLightMapPolicySkylight") };
        static const TSet<FString> PaperVS = { TEXT("FHitProxyVS"), TEXT("TLightMapDensityVSFNoLightMapPolicy"), TEXT("TDepthOnlyVS<false>"), TEXT("FVelocityVS"), TEXT("TShadowDepthVSVertexShadowDepth_PerspectiveCorrectfalse"), TEXT("TShadowDepthVSVertexShadowDepth_OutputDepthfalse"), TEXT("TShadowDepthVSVertexShadowDepth_OnePassPointLightfalse"), TEXT("TShadowDepthVSForGSVertexShadowDepth_OnePassPointLight") };
        static const TSet<FString> PaperPS = { TEXT("FHitProxyPS"), TEXT("TLightMapDensityPSFNoLightMapPolicy"), TEXT("FVelocityPS"), TEXT("FDepthOnlyPS<false>"), TEXT("TShadowDepthPSPixelShadowDepth_PerspectiveCorrectfalse"), TEXT("TShadowDepthPSPixelShadowDepth_OnePassPointLightfalse"), TEXT("TShadowDepthPSPixelShadowDepth_NonPerspectiveCorrectfalse") };
        // Only shader types present in the verified M_CommonLandscape fixture.
        static const TSet<FString> LandscapeVS = {
            TEXT("FHitProxyVS"), TEXT("FLandscapeGrassWeightVS"), TEXT("TLightMapDensityVSFNoLightMapPolicy"), TEXT("TLightMapDensityVSFDummyLightMapPolicy"),
            TEXT("TDepthOnlyVS<false>"), TEXT("FVelocityVS"), TEXT("TShadowDepthVSVertexShadowDepth_PerspectiveCorrectfalse"), TEXT("TShadowDepthVSVertexShadowDepth_OutputDepthfalse"),
            TEXT("TShadowDepthVSVertexShadowDepth_OnePassPointLightfalse"), TEXT("TShadowDepthVSForGSVertexShadowDepth_OnePassPointLight"),
            TEXT("TBasePassVSFSimpleDirectionalLightLightingPolicy"), TEXT("TVirtualTextureVSWorldHeight"), TEXT("TVirtualTextureVSBaseColor"), TEXT("TVirtualTextureVSBaseColorNormalSpecular") };
        static const TSet<FString> LandscapePS = {
            TEXT("FHitProxyPS"), TEXT("FLandscapeGrassWeightPS"), TEXT("TLightMapDensityPSFNoLightMapPolicy"), TEXT("TLightMapDensityPSFDummyLightMapPolicy"),
            TEXT("FVelocityPS"), TEXT("FDepthOnlyPS<false>"), TEXT("TShadowDepthPSPixelShadowDepth_PerspectiveCorrectfalse"),
            TEXT("TBasePassPSFSimpleDirectionalLightLightingPolicy"), TEXT("TBasePassPSFSimpleDirectionalLightLightingPolicySkylight"),
            TEXT("TVirtualTexturePSWorldHeight"), TEXT("TVirtualTexturePSBaseColor"), TEXT("TVirtualTexturePSBaseColorNormalSpecular") };
        bool VS = R.Type.StartsWith(TEXT("TSlateMaterialShaderVS")) || R.Type.StartsWith(TEXT("TTranslucencyShadowDepthVS<")) || R.Type.StartsWith(TEXT("TBasePassVS")) || R.Type == TEXT("FConvertToUniformMeshVS") || ((bPaper2D && PaperVS.Contains(R.Type)) || (bLandscape && LandscapeVS.Contains(R.Type)));
        bool PS = R.Type.StartsWith(TEXT("TSlateMaterialShaderPS")) || R.Type.StartsWith(TEXT("TTranslucencyShadowDepthPS<")) || R.Type.StartsWith(TEXT("TBasePassPS")) || ((bPaper2D && PaperPS.Contains(R.Type)) || (bLandscape && LandscapePS.Contains(R.Type)));
        bool GS = R.Type == TEXT("FConvertToUniformMeshGS") || ((bPaper2D || bLandscape) && R.Type == TEXT("FOnePassPointShadowDepthGS"));
        if (!(Allowed.Contains(R.Type) || (bPaper2D && (PaperVS.Contains(R.Type) || PaperPS.Contains(R.Type) || R.Type == TEXT("FOnePassPointShadowDepthGS"))) || (bLandscape && (LandscapeVS.Contains(R.Type) || LandscapePS.Contains(R.Type) || R.Type == TEXT("FOnePassPointShadowDepthGS")))) || (!VS && !PS && !GS) || End <= P || End > Limit || (Mesh == R.Type.StartsWith(TEXT("TSlateMaterialShader")))) { Fail(TEXT("invalid shader envelope")); return; }
        int32 Outer=Limit; Limit=int32(End);
        R.SceneTextures=Uniform(); R.Material=Uniform(); for (int32 I=Count(); I-- > 0;) R.CollectionBuffers.Add(Uniform());
        for (int32 I=0; I<7; ++I) if (Count()!=Counts[I]) Fail(TEXT("expression count mismatch"));
        if (Name()!=FName(TEXT("Material"))) Fail(TEXT("invalid shader layout name"));
        R.LayoutSize=Read<uint32>(); for(int32 I=Count(); I-- > 0;) R.LayoutOffsets.Add(Read<uint16>());
        int32 TypeCount=Count(); for(int32 I=0; I<TypeCount; ++I) R.LayoutTypes.Add(Read<uint8>());
        if (R.LayoutSize>65536 || TypeCount!=R.LayoutOffsets.Num()) Fail(TEXT("invalid debug layout"));
        for(uint16 Offset:R.LayoutOffsets) if(uint32(Offset)+8>R.LayoutSize) Fail(TEXT("invalid debug offset"));
        R.DebugDescription=String(); R.VTFeedback=Param(true);
        if (!Mesh) { for(int32 I=0; I<(VS?2:3); ++I) R.AdditionalParameters.Add(Param()); if(PS) { R.AdditionalParameters.Add(Param(true)); R.AdditionalParameters.Add(Param(true)); } }
        else
        {
            R.Pass=Uniform(); FName V=Name(); uint8 F=Read<uint8>(), Platform=Read<uint8>(); R.VFParameterSourceHash=Hash(); int64 VEnd=Read<int64>();
            if(V!=VF || F!=(VS?0:PS?3:4) || Platform!=0 || VEnd<P || VEnd>Limit) Fail(TEXT("invalid vertex factory identity"));
            if(VS && VF==FName(TEXT("FLocalVertexFactory"))) { uint32 SpeedTree=Read<uint32>(); if(SpeedTree>1) Fail(TEXT("invalid speedtree flag")); R.bSpeedTree=SpeedTree!=0; R.VFLod=Param(); }
            else if(VS && bLandscape)
            {
                R.LandscapeLodValues=Param(); R.LandscapeForcedLod=Param(); R.LandscapeLodTessellation=Param();
                R.LandscapeNeighborSectionLod=Param(); R.LandscapeLodBias=Param(); R.LandscapeSectionLods=Param();
            }
            if(P!=VEnd) Fail(TEXT("vertex factory boundary mismatch"));
            if(R.Type.StartsWith(TEXT("TBasePass"))) for(int32 I=0;I<4;++I) R.AdditionalUniforms.Add(Uniform());
            else if(R.Type.StartsWith(TEXT("TTranslucencyShadowDepthPS"))) R.AdditionalParameters.Add(Param());
            else if((bPaper2D || bLandscape) && R.Type.StartsWith(TEXT("TLightMapDensity"))) { for(int32 I=0;I<3;++I) R.AdditionalUniforms.Add(Uniform()); if(PS) for(int32 I=0;I<3;++I) R.AdditionalParameters.Add(Param()); }
            else if((bPaper2D || bLandscape) && R.Type.StartsWith(TEXT("TShadowDepthPS"))) for(int32 I=0;I<8;++I) R.AdditionalParameters.Add(Param(true));
            else if((bPaper2D || bLandscape) && (R.Type.StartsWith(TEXT("TShadowDepthVS")) || R.Type==TEXT("FDepthOnlyPS<false>") || R.Type==TEXT("FHitProxyPS"))) R.AdditionalParameters.Add(Param());
            else if((bPaper2D || bLandscape) && R.Type==TEXT("FHitProxyVS")) R.AdditionalParameters.Add(Param(true));
            else if(bLandscape && (R.Type==TEXT("FLandscapeGrassWeightVS") || R.Type==TEXT("FLandscapeGrassWeightPS"))) R.AdditionalParameters.Add(Param());
        }
        R.OutputHash=Hash(); if(Hash()!=O.IR.OriginalMapHash) Fail(TEXT("map hash mismatch"));
        R.Pipeline=Name(); R.VertexFactory=Name(); if(Mesh && R.VertexFactory!=VF) Fail(TEXT("vertex factory mismatch")); R.VertexFactorySourceHash=Hash();
        if((bPaper2D || bLandscape) && R.Pipeline!=ExpectedPipeline) Fail(TEXT("shader pipeline identity mismatch"));
        if(Name()!=Type) Fail(TEXT("shader type mismatch")); R.PermutationId=Read<int32>(); R.SourceHash=Hash(); R.Frequency=Read<uint32>(); uint32 Platform=Read<uint32>();
        if(R.Frequency!=(VS?0u:PS?3u:4u) || Platform) Fail(TEXT("shader target mismatch"));
        for(int32 I=Count(); I-- > 0;) { FLegacyOriginalNamedUniform B; B.Name=Name(); B.Binding=Uniform(); R.UniformParameters.Add(B); }
        R.ResourceType=Name(); R.ResourcePermutationId=Read<int32>(); uint32 RF=Read<uint32>(), RP=Read<uint32>(); FSHAHash RH=Hash();
        if(RF!=R.Frequency || RP || RH!=R.OutputHash) Fail(TEXT("shader resource mismatch"));
        R.InstructionCount=Read<uint32>(); for(int32 I=Count();I-- > 0;) R.UniformBuffers.Add(Pair()); for(int32 I=Count();I-- > 0;) R.Samplers.Add(Pair()); for(int32 I=Count();I-- > 0;) R.SRVs.Add(Pair());
        for(int32 I=Count();I-- > 0;) { FLegacyOriginalLooseBuffer B; B.Header=Pair(); for(int32 J=Count();J-- > 0;) B.Parameters.Add(Pair()); R.LooseBuffers.Add(MoveTemp(B)); }
        R.UncompressedBytes=Read<uint32>(); if(R.UncompressedBytes>64*1024*1024 || Read<uint32>()!=1) Fail(TEXT("invalid library resource"));
        for(int32 I=0;I<3;++I) if(Count()) Fail(TEXT("unsupported parameter bindings")); R.BindingWord=Read<uint32>(); R.RootParameterBufferIndex=Read<uint16>();
        if(P!=End) Fail(TEXT("shader boundary mismatch")); R.End=P; O.Records.Add(MoveTemp(R)); Limit=Outer;
    }
    void Records(bool Mesh=false, FName VF=NAME_None)
    {
        for(int32 I=Count();I-- > 0;) Record(Mesh,VF);
        int32 Pipelines=Count();
        if((!bPaper2D && !bLandscape) || !Mesh) { if(Pipelines) Fail(TEXT("unsupported shader pipelines")); return; }
        TSet<FName> Seen;
        for(int32 I=0; I<Pipelines && bOk; ++I)
        {
            FLegacyOriginalPipeline Pipeline; Pipeline.Begin=P; Pipeline.Type=Name(); Pipeline.VertexFactory=VF;
            TArray<FName> Expected;
            if(Pipeline.Type==FName(TEXT("DepthNoColorOutputPipeline"))) { Expected.Add(FName(TEXT("FDepthOnlyPS<false>"))); Expected.Add(FName(TEXT("TDepthOnlyVS<false>"))); }
            else if(Pipeline.Type==FName(TEXT("DepthNoPixelPipeline"))) Expected.Add(FName(TEXT("TDepthOnlyVS<false>")));
            else if(Pipeline.Type==FName(TEXT("VelocityPipeline"))) { Expected.Add(FName(TEXT("FVelocityPS"))); Expected.Add(FName(TEXT("FVelocityVS"))); }
            else { Fail(TEXT("unsupported shader pipeline")); return; }
            if(Seen.Contains(Pipeline.Type) || Count()!=Expected.Num()) { Fail(TEXT("invalid shader pipeline stages")); return; }
            Seen.Add(Pipeline.Type);
            for(FName Stage:Expected) { int32 Index=O.Records.Num(); Record(true,VF,Pipeline.Type); if(!bOk) return; if(O.Records[Index].Type!=Stage.ToString()) { Fail(TEXT("shader pipeline stage mismatch")); return; } Pipeline.StageIndices.Add(Index); }
            Pipeline.End=P; O.Pipelines.Add(MoveTemp(Pipeline));
        }
    }
    bool Run()
    {
        if(O.Bytes.Num()>64*1024*1024 || Read<uint32>()!=1 || Read<uint32>()!=1) return Fail(TEXT("invalid resource header"));
        uint32 Q=Read<uint32>(), F=Read<uint32>(); if(Q>5 || F>6) Fail(TEXT("invalid quality or feature"));
        O.IR.QualityLevel=static_cast<EMaterialQualityLevel::Type>(Q); O.IR.FeatureLevel=static_cast<ERHIFeatureLevel::Type>(F);
        O.IR.OriginalMapHash=Hash(); if(Read<uint32>()!=0) Fail(TEXT("unsupported original platform"));
        O.FriendlyName=String(); O.MaterialPath=String(); O.IR.DebugDescription=O.FriendlyName;
        bPaper2D=O.MaterialPath==TEXT("/Paper2D/DefaultSpriteMaterial.DefaultSpriteMaterial") && O.FriendlyName==TEXT("DefaultSpriteMaterial");
        bLandscape=O.MaterialPath==TEXT("/Game/Athena/Apollo/Environments/Landscape/Materials/Biome/LandscapeBiomeMaterials/M_CommonLandscape.M_CommonLandscape") && O.FriendlyName==TEXT("M_CommonLandscape");
        if(!bPaper2D && (!O.MaterialPath.StartsWith(TEXT("/Game/")) || !O.MaterialPath.EndsWith(TEXT(".")+O.FriendlyName))) Fail(TEXT("invalid material identity"));
        for(int32 A=0; A<8; ++A)
        {
            if(A==7)
            {
                const int32 StackCount=Count();
                if(StackCount>128) Fail(TEXT("too many VT stacks"));
                for(int32 S=0; S<StackCount && bOk; ++S)
                {
                    FLegacyMaterialVTStack Stack;
                    const uint32 Layers=Read<uint32>();
                    if(Layers>8) { Fail(TEXT("unsupported VT stack layer count")); break; }
                    for(uint32 L=0; L<Layers; ++L) Stack.LayerUniformExpressionIndices.Add(Read<int32>());
                    Stack.PreallocatedStackTextureIndex=Read<int32>();
                    O.IR.VTStacks.Add(MoveTemp(Stack));
                }
            }
            int32 N=Count(); if(A>2 && A!=5 && N) Fail(TEXT("unsupported texture root array")); if(A<4) Counts[A]=N; else if(A==4) Counts[5]=N; else if(A==5) Counts[6]=N; else if(A==7) Counts[4]=N;
            for(int32 I=0;I<N;++I) { int32 X=Expression(0, A==2 || A==5, A==0, A==5); if(A==0) O.IR.VectorRoots.Add(X); else if(A==1) O.IR.ScalarRoots.Add(X); }
        }
        for (const auto& Pending : PendingTextureSizes)
        {
            int32 Match=INDEX_NONE, Matches=0;
            for (int32 I=0; I<O.IR.Texture2DParameters.Num(); ++I)
            {
                const auto& A=Pending.Texture; const auto& B=O.IR.Texture2DParameters[I];
                if (A.Parameter==B.Parameter && A.TextureIndex==B.TextureIndex && A.TextureLayerIndex==B.TextureLayerIndex &&
                    A.PageTableLayerIndex==B.PageTableLayerIndex && A.SamplerSource==B.SamplerSource && A.bVirtualTexture==B.bVirtualTexture)
                { Match=I; ++Matches; }
            }
            if (Matches!=1 || !O.IR.Expressions.IsValidIndex(Pending.Node))
                return Fail(TEXT("texture-size child has no unambiguous Standard2D slot"));
            O.IR.Expressions[Pending.Node].TextureParameterIndex=Match;
        }
        for(int32 I=Count(); I-- > 0;) for(int32 J=0; J<4; ++J) Read<uint32>();
        O.UsedSceneTextures=Read<uint32>(); O.TextureSamplesVS=Read<uint16>(); O.TextureSamplesPS=Read<uint16>(); O.VirtualTextureLookups=Read<uint16>(); O.UVScalars=Read<uint8>(); O.CustomInterpolatorScalars=Read<uint8>(); O.VirtualTextureMask=Read<uint8>(); O.Flags=Read<uint8>();
        O.IR.UsedSceneTextures=O.UsedSceneTextures; O.IR.EstimatedTextureSamplesVS=O.TextureSamplesVS; O.IR.EstimatedTextureSamplesPS=O.TextureSamplesPS; O.IR.EstimatedVirtualTextureLookups=O.VirtualTextureLookups; O.IR.UsedUVScalars=O.UVScalars; O.IR.UsedCustomInterpolatorScalars=O.CustomInterpolatorScalars; O.IR.RuntimeVirtualTextureOutputAttributeMask=O.VirtualTextureMask; O.IR.CompilationFlags=O.Flags;
        if(O.Flags&0x80) Fail(TEXT("invalid compilation flags"));
        O.IR.DebugDescription=String(); Records(); int32 Meshes=Count();
        for(int32 I=0;I<Meshes;++I) { FName VF=Name();
            const bool bLandscapeVF=VF==FName(TEXT("FLandscapeVertexFactory")) || VF==FName(TEXT("FLandscapeXYOffsetVertexFactory")) || VF==FName(TEXT("FLandscapeFixedGridVertexFactory"));
            if(bLandscape ? !bLandscapeVF : VF!=FName(TEXT("FLocalVertexFactory"))) Fail(TEXT("unsupported vertex factory")); Records(true,VF); }
        if(Read<uint32>()!=1 || P!=Limit) Fail(TEXT("resource boundary mismatch"));
        return bOk;
    }
};
}
bool DecodeLegacyOriginalMaterial(FLegacyOriginalMaterial& Out, FString& Error) { return FReader(Out, Error).Run(); }
#endif
