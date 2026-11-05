#include "LegacyOriginalMaterialLayout.h"

#if PLATFORM_LINUX && !UE_SERVER
bool PrepareLegacyOriginalMaterialLayout(const FLegacyOriginalMaterial& Original,
    FLegacyMaterialIR& Out, FString& Error)
{
    auto Fail = [&Error](const TCHAR* Message) { Error=Message; return false; };
    const auto& IR=Original.IR;
    if (!Original.Records.Num() || IR.VectorRoots.Num()>1024 || IR.ScalarRoots.Num()>1024 ||
        IR.Texture2DParameters.Num()>128 || IR.VirtualTextureParameters.Num()>128 || IR.VTStacks.Num()>128)
        return Fail(TEXT("invalid original material expression shape"));

    // Original debug layouts serialize resource types as 5=texture, 6=SRV,
    // 8=sampler. Do not assume that original enum ordinals match this engine.
    struct FExpected { uint8 OriginalType; EUniformBufferBaseType CurrentType; };
    TArray<FExpected> Expected;
    auto Add = [&Expected](uint8 OriginalType, EUniformBufferBaseType CurrentType)
    { Expected.Add({OriginalType,CurrentType}); };
    for (int32 I=0; I<IR.Texture2DParameters.Num(); ++I) { Add(5,UBMT_TEXTURE); Add(8,UBMT_SAMPLER); }
    for (const auto& Stack:IR.VTStacks)
    {
        if (Stack.LayerUniformExpressionIndices.Num()<1 || Stack.LayerUniformExpressionIndices.Num()>8)
            return Fail(TEXT("invalid original material VT stack"));
        Add(5,UBMT_TEXTURE);
        if (Stack.LayerUniformExpressionIndices.Num()>4) Add(5,UBMT_TEXTURE);
    }
    for (int32 I=0; I<IR.VirtualTextureParameters.Num(); ++I) { Add(6,UBMT_SRV); Add(8,UBMT_SAMPLER); }
    Add(8,UBMT_SAMPLER); Add(8,UBMT_SAMPLER);
    const uint32 NumericBytes=uint32(IR.VTStacks.Num())*32u + uint32(IR.VirtualTextureParameters.Num())*16u +
        uint32(IR.VectorRoots.Num())*16u + ((uint32(IR.ScalarRoots.Num())+3u)/4u)*16u;
    const uint32 Size=Align(NumericBytes+uint32(Expected.Num())*8u,16u);
    if (Size>65535) return Fail(TEXT("original material layout is too large"));
    for (const auto& Record:Original.Records)
    {
        if (Record.LayoutSize!=Size || Record.LayoutOffsets.Num()!=Expected.Num() || Record.LayoutTypes.Num()!=Expected.Num())
            return Fail(TEXT("original material debug layout size or count mismatch"));
        for (int32 I=0; I<Expected.Num(); ++I)
            if (Record.LayoutOffsets[I]!=NumericBytes+uint32(I)*8u || Record.LayoutTypes[I]!=Expected[I].OriginalType)
                return Fail(TEXT("original material debug resource layout mismatch"));
    }
    FLegacyMaterialIR Result=IR;
    Result.UniformBufferSize=Size;
    Result.Resources.Reset(Expected.Num());
    for (int32 I=0; I<Expected.Num(); ++I)
    {
        FLegacyMaterialResource Resource;
        Resource.Offset=uint16(NumericBytes+uint32(I)*8u);
        Resource.BaseType=Expected[I].CurrentType;
        Result.Resources.Add(Resource);
    }
    Out=MoveTemp(Result);
    Error.Reset();
    return true;
}
#endif
