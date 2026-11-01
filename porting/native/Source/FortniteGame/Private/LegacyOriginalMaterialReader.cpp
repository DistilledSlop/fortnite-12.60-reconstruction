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
    bool bOk = true;
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
    void Uniform() { Read<uint16>(); if (Read<uint32>() > 1) Fail(TEXT("invalid uniform binding")); }
    void Param(bool Resource = false) { Read<uint16>(); Read<uint16>(); if (!Resource) Read<uint16>(); }
    void ParameterInfo(FLegacyMaterialExpression& E)
    {
        FName N = Name(); uint8 A = Read<uint8>(); int32 I = Read<int32>();
        if (A > 2 || I < -1) Fail(TEXT("invalid parameter identity"));
        E.Parameter = FMaterialParameterInfo(N, static_cast<EMaterialParameterAssociation>(A), I);
    }
    int32 Expression(int32 Depth, bool bTextureRoot = false)
    {
        if (Depth >= 64 || ++Nodes > 65536) { Fail(TEXT("expression complexity")); return INDEX_NONE; }
        FString T = Name().ToString(); const FString Prefix(TEXT("FMaterialUniformExpression"));
        if (!T.RemoveFromStart(Prefix)) { Fail(TEXT("invalid expression tag")); return INDEX_NONE; }
        FLegacyMaterialExpression E;
        if (bTextureRoot && T != TEXT("TextureParameter")) Fail(TEXT("invalid texture root"));
        if (T == TEXT("Constant")) { E.Kind = ELegacyMaterialExpression::Constant; E.Value.R=Read<float>(); E.Value.G=Read<float>(); E.Value.B=Read<float>(); E.Value.A=Read<float>(); E.ValueType=Read<uint8>(); }
        else if (T == TEXT("ScalarParameter") || T == TEXT("VectorParameter"))
        {
            E.Kind = T == TEXT("ScalarParameter") ? ELegacyMaterialExpression::ScalarParameter : ELegacyMaterialExpression::VectorParameter;
            ParameterInfo(E); E.Value.R=Read<float>();
            if (T == TEXT("VectorParameter")) { E.Value.G=Read<float>(); E.Value.B=Read<float>(); E.Value.A=Read<float>(); }
        }
        else if (T == TEXT("TextureParameter"))
        {
            ParameterInfo(E); int32 Index=Read<int32>(); int16 Layer=Read<int16>(), Page=Read<int16>(); int32 Sampler=Read<int32>(); uint32 Virtual=Read<uint32>();
            if (Index < -1 || Layer < -1 || Page < -1 || Sampler < 0 || Sampler > 2 || Virtual > 1) Fail(TEXT("invalid texture expression"));
            if (!bTextureRoot || Depth != 0 || Index < 0 || Layer != -1 || Page != -1 || Virtual != 0)
                Fail(TEXT("unsupported texture expression"));
            FLegacyMaterialTextureParameter Texture;
            Texture.Parameter=E.Parameter; Texture.TextureIndex=Index; Texture.TextureLayerIndex=Layer;
            Texture.PageTableLayerIndex=Page; Texture.SamplerSource=uint8(Sampler); Texture.bVirtualTexture=Virtual != 0;
            O.IR.Texture2DParameters.Add(MoveTemp(Texture));
            return INDEX_NONE;
        }
        else if (T == TEXT("Max") || T == TEXT("Saturate"))
        {
            E.A=Expression(Depth+1);
            if (T == TEXT("Max")) { E.B=Expression(Depth+1); E.Kind=ELegacyMaterialExpression::Max; }
            else E.Kind=ELegacyMaterialExpression::Saturate;
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
    void Record(bool Mesh, FName VF)
    {
        FLegacyOriginalRecord R; R.Begin=P; FName Type=Name(); R.Type=Type.ToString(); int64 End=Read<int64>();
        static const TSet<FString> Allowed = {
            TEXT("TSlateMaterialShaderVStrue"), TEXT("TSlateMaterialShaderVSfalse"),
            TEXT("TSlateMaterialShaderPSGrayscaleFont"), TEXT("TSlateMaterialShaderPSDefault"), TEXT("TSlateMaterialShaderPSCustom"), TEXT("TSlateMaterialShaderPSColorFont"), TEXT("TSlateMaterialShaderPSBorder"),
            TEXT("TTranslucencyShadowDepthVS<TranslucencyShadowDepth_Standard>"), TEXT("TTranslucencyShadowDepthVS<TranslucencyShadowDepth_PerspectiveCorrect>"),
            TEXT("TTranslucencyShadowDepthPS<TranslucencyShadowDepth_Standard>"), TEXT("TTranslucencyShadowDepthPS<TranslucencyShadowDepth_PerspectiveCorrect>"),
            TEXT("TBasePassVSFSimpleNoLightmapLightingPolicy"), TEXT("TBasePassVSFNoLightMapPolicy"), TEXT("FConvertToUniformMeshVS"), TEXT("FConvertToUniformMeshGS"),
            TEXT("TBasePassPSFSimpleNoLightmapLightingPolicy"), TEXT("TBasePassPSFSimpleNoLightmapLightingPolicySkylight"),
            TEXT("TBasePassPSFNoLightMapPolicy"), TEXT("TBasePassPSFNoLightMapPolicySkylight") };
        bool VS = R.Type.StartsWith(TEXT("TSlateMaterialShaderVS")) || R.Type.StartsWith(TEXT("TTranslucencyShadowDepthVS<")) || R.Type.StartsWith(TEXT("TBasePassVS")) || R.Type == TEXT("FConvertToUniformMeshVS");
        bool PS = R.Type.StartsWith(TEXT("TSlateMaterialShaderPS")) || R.Type.StartsWith(TEXT("TTranslucencyShadowDepthPS<")) || R.Type.StartsWith(TEXT("TBasePassPS"));
        bool GS = R.Type == TEXT("FConvertToUniformMeshGS");
        if (!Allowed.Contains(R.Type) || (!VS && !PS && !GS) || End <= P || End > Limit || (Mesh == R.Type.StartsWith(TEXT("TSlateMaterialShader")))) { Fail(TEXT("invalid shader envelope")); return; }
        int32 Outer=Limit; Limit=int32(End);
        Uniform(); Uniform(); for (int32 I=Count(); I-- > 0;) Uniform();
        for (int32 I=0; I<7; ++I) if (Count()!=Counts[I]) Fail(TEXT("expression count mismatch"));
        if (Name()!=FName(TEXT("Material"))) Fail(TEXT("invalid shader layout name"));
        uint32 Size=Read<uint32>(); TArray<uint16> Offsets; for(int32 I=Count(); I-- > 0;) Offsets.Add(Read<uint16>());
        int32 TypeCount=Count(); for(int32 I=0; I<TypeCount; ++I) Read<uint8>();
        if (Size>65536 || TypeCount!=Offsets.Num()) Fail(TEXT("invalid debug layout"));
        for(uint16 Offset:Offsets) if(uint32(Offset)+8>Size) Fail(TEXT("invalid debug offset"));
        String(); Param(true);
        if (!Mesh) { for(int32 I=0; I<(VS?2:3); ++I) Param(); if(PS) { Param(true); Param(true); } }
        else
        {
            Uniform(); FName V=Name(); uint8 F=Read<uint8>(), Platform=Read<uint8>(); Hash(); int64 VEnd=Read<int64>();
            if(V!=VF || F!=(VS?0:PS?3:4) || Platform!=0 || VEnd<P || VEnd>Limit) Fail(TEXT("invalid vertex factory identity"));
            if(VS) { if(Read<uint32>()>1) Fail(TEXT("invalid speedtree flag")); Param(); }
            if(P!=VEnd) Fail(TEXT("vertex factory boundary mismatch"));
            if(R.Type.StartsWith(TEXT("TBasePass"))) for(int32 I=0;I<4;++I) Uniform();
            else if(R.Type.StartsWith(TEXT("TTranslucencyShadowDepthPS"))) Param();
        }
        R.OutputHash=Hash(); if(Hash()!=O.IR.OriginalMapHash) Fail(TEXT("map hash mismatch"));
        Name(); FName RecordVF=Name(); if(Mesh && RecordVF!=VF) Fail(TEXT("vertex factory mismatch")); Hash();
        if(Name()!=Type) Fail(TEXT("shader type mismatch")); Read<int32>(); Hash(); R.Frequency=Read<uint32>(); uint32 Platform=Read<uint32>();
        if(R.Frequency!=(VS?0u:PS?3u:4u) || Platform) Fail(TEXT("shader target mismatch"));
        for(int32 I=Count(); I-- > 0;) { Name(); Uniform(); }
        Name(); Read<int32>(); uint32 RF=Read<uint32>(), RP=Read<uint32>(); FSHAHash RH=Hash();
        if(RF!=R.Frequency || RP || RH!=R.OutputHash) Fail(TEXT("shader resource mismatch"));
        Read<uint32>(); for(int32 A=0;A<3;++A) for(int32 I=Count();I-- > 0;) { Read<uint16>(); Read<uint16>(); }
        for(int32 I=Count();I-- > 0;) { Read<uint16>(); Read<uint16>(); for(int32 J=Count();J-- > 0;) { Read<uint16>(); Read<uint16>(); } }
        R.UncompressedBytes=Read<uint32>(); if(R.UncompressedBytes>64*1024*1024 || Read<uint32>()!=1) Fail(TEXT("invalid library resource"));
        for(int32 I=0;I<3;++I) if(Count()) Fail(TEXT("unsupported parameter bindings")); Read<uint32>(); Read<uint16>();
        if(P!=End) Fail(TEXT("shader boundary mismatch")); R.End=P; O.Records.Add(MoveTemp(R)); Limit=Outer;
    }
    void Records(bool Mesh=false, FName VF=NAME_None) { for(int32 I=Count();I-- > 0;) Record(Mesh,VF); if(Count()) Fail(TEXT("unsupported shader pipelines")); }
    bool Run()
    {
        if(O.Bytes.Num()>64*1024*1024 || Read<uint32>()!=1 || Read<uint32>()!=1) return Fail(TEXT("invalid resource header"));
        uint32 Q=Read<uint32>(), F=Read<uint32>(); if(Q>5 || F>6) Fail(TEXT("invalid quality or feature"));
        O.IR.QualityLevel=static_cast<EMaterialQualityLevel::Type>(Q); O.IR.FeatureLevel=static_cast<ERHIFeatureLevel::Type>(F);
        O.IR.OriginalMapHash=Hash(); if(Read<uint32>()!=0) Fail(TEXT("unsupported original platform"));
        O.FriendlyName=String(); O.MaterialPath=String(); O.IR.DebugDescription=O.FriendlyName;
        if(!O.MaterialPath.StartsWith(TEXT("/Game/")) || !O.MaterialPath.EndsWith(TEXT(".")+O.FriendlyName)) Fail(TEXT("invalid material identity"));
        for(int32 A=0; A<8; ++A)
        {
            if(A==7 && Count()!=0) Fail(TEXT("unsupported VT stacks"));
            int32 N=Count(); if(A>2 && N) Fail(TEXT("unsupported texture root array")); if(A<6) Counts[A]=N; else if(A==7) Counts[6]=N;
            for(int32 I=0;I<N;++I) { int32 X=Expression(0, A==2); if(A==0) O.IR.VectorRoots.Add(X); else if(A==1) O.IR.ScalarRoots.Add(X); }
        }
        for(int32 I=Count(); I-- > 0;) for(int32 J=0; J<4; ++J) Read<uint32>();
        O.UsedSceneTextures=Read<uint32>(); O.TextureSamplesVS=Read<uint16>(); O.TextureSamplesPS=Read<uint16>(); O.VirtualTextureLookups=Read<uint16>(); O.UVScalars=Read<uint8>(); O.CustomInterpolatorScalars=Read<uint8>(); O.VirtualTextureMask=Read<uint8>(); O.Flags=Read<uint8>();
        O.IR.UsedSceneTextures=O.UsedSceneTextures; O.IR.EstimatedTextureSamplesVS=O.TextureSamplesVS; O.IR.EstimatedTextureSamplesPS=O.TextureSamplesPS; O.IR.EstimatedVirtualTextureLookups=O.VirtualTextureLookups; O.IR.UsedUVScalars=O.UVScalars; O.IR.UsedCustomInterpolatorScalars=O.CustomInterpolatorScalars; O.IR.RuntimeVirtualTextureOutputAttributeMask=O.VirtualTextureMask; O.IR.CompilationFlags=O.Flags;
        if(O.Flags&0x80) Fail(TEXT("invalid compilation flags"));
        O.IR.DebugDescription=String(); Records(); int32 Meshes=Count();
        for(int32 I=0;I<Meshes;++I) { FName VF=Name(); if(VF!=FName(TEXT("FLocalVertexFactory"))) Fail(TEXT("unsupported vertex factory")); Records(true,VF); }
        if(Read<uint32>()!=1 || P!=Limit) Fail(TEXT("resource boundary mismatch"));
        return bOk;
    }
};
}
bool DecodeLegacyOriginalMaterial(FLegacyOriginalMaterial& Out, FString& Error) { return FReader(Out, Error).Run(); }
#endif
