// Typed review draft of 0x141ddae00. NOT integrated or build verified.
// All FEffects methods require implementations; see evidence.json before use.
#pragma once
#include "AFortWindManager.h"
#include "ABuildingSMActor.h"
#include "FFortWindResponder.h"
#include "Curves/CurveLinearColor.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace FortWindMaterialSetupDraft
{
// Distinct call sites intentionally retain missing native arguments as a closure gap.
enum class EMIDCall { FirstCall, SecondCall };
struct FEffects
{
    virtual ~FEffects() = default;
    virtual UMeshComponent* Mesh_141c07130(ABuildingSMActor*) = 0;
    virtual UMaterialInstanceDynamic* GetOrCreateMID_141c0a630(
        ABuildingSMActor*, UMeshComponent*, int32 Slot, EMIDCall) = 0;
    virtual UMaterialInstanceDynamic* CreateIntenseMID_1442ad650(
        ABuildingSMActor*, UMaterialInterface*) = 0;
    // 141dccf40 is only known here by non-null result; null MID must be supported.
    virtual bool HasScalar_141dccf40(AFortWindManager&, UMaterialInstanceDynamic*, FName) = 0;
    virtual UMaterialInterface* BaseMaterial_141b3a3e0(UMaterialInstanceDynamic*) = 0;
    // Search each MI override array, then parent only while it remains an MI.
    // Do not replace with a fully resolved getter: absence triggers base fallback.
    virtual bool FindScalarOverrideInMIChain(UMaterialInstanceDynamic*, FName, float&) = 0;
    virtual bool FindVectorOverrideInMIChain(UMaterialInstanceDynamic*, FName, FLinearColor&) = 0;
    // Original uses common base virtual slots 0x3e0/0x3f8 and parameter info
    // association byte 2, index -1. Missing return/out args require ABI recovery.
    virtual float BaseScalar_3e0(UMaterialInterface*, FName) = 0;
    virtual FLinearColor BaseVector_3f8(UMaterialInterface*, FName) = 0;
    // Native 144333900/9a0 initialize values AND cache parameter indices.
    // Hidden value/out args must be recovered; role-specific hooks prevent
    // guessing values for interpolation initialization from truncated calls.
    virtual void InitializePairScalar_144333900(UMaterialInstanceDynamic*, FName, int32&) = 0;
    virtual void InitializeInterpolatedScalar_144333900(UMaterialInstanceDynamic*, FName,
        float From, float To, int32&) = 0;
    virtual void InitializeInterpolatedVector_1443339a0(UMaterialInstanceDynamic*, FName,
        const FLinearColor& From, const FLinearColor& To, int32&) = 0;
    virtual void InitializeWindVector_1443339a0(UMaterialInstanceDynamic*, FName,
        const FLinearColor& CapturedInitialValue, int32&) = 0;
    // Final calls omit float arguments in capture. Speed and offset are inferred
    // from freshly initialized pair fields; implement only after assembly check.
    virtual void InitializePairWrites_14433ac70(UMaterialInstanceDynamic*,
        int32 SpeedIndex, int32 OffsetIndex, float Speed, float Offset) = 0;
    virtual void LogMissingMild(ABuildingSMActor*, UMeshComponent*, int32) = 0;
    virtual void LogMIDFailure(ABuildingSMActor*, int32) = 0;
    virtual void LogBothMIDNull(ABuildingSMActor*, int32) = 0;
    virtual void LogUnsupportedPair(ABuildingSMActor*) = 0;
};

inline float Channel(const FLinearColor& V, int32 I)
{
    switch (I) {
    case 0: case 4: return V.R;
    case 1: case 5: return V.G;
    case 2: case 6: return V.B;
    case 3: case 7: return V.A;
    default: return 0.0f;
    }
}
inline bool EqualComponents(const FLinearColor& A, const FLinearColor& B)
{
    return A.R == B.R && A.G == B.G && A.B == B.B && A.A == B.A;
}
inline void Setup(AFortWindManager& M, FFortWindResponder& R, FEffects& E)
{
    UMeshComponent* Mesh = E.Mesh_141c07130(R.WindUpdatingBuildingSMActor);
    if (!Mesh) { R.bHasSetupAnimatingMaterials = true; return; }
    const int32 NumSlots = Mesh->GetNumMaterials();
    for (int32 Slot = 0; Slot < NumSlots; ++Slot)
    {
        FFortWindMaterialData D{};
        D.MaterialParameterPairIndices = 0;
        D.WindVectorParameterIndex = INDEX_NONE;
        // Native indexes this array directly: constructor must supply one entry
        // per mesh slot. Do not silently clamp to array length.
        UMaterialInterface* Mild = R.MildWindMaterialInstances[Slot];
        if (!Mild) E.LogMissingMild(R.WindUpdatingBuildingSMActor, Mesh, Slot);
        else
        {
            UMaterialInstanceDynamic* MildMID = E.GetOrCreateMID_141c0a630(
                R.WindUpdatingBuildingSMActor, Mesh, Slot, EMIDCall::FirstCall);
            if (!MildMID) E.LogMIDFailure(R.WindUpdatingBuildingSMActor, Slot);
            if (Slot < R.IntenseWindMaterialInstances.Num())
                D.IntenseStateMID = E.CreateIntenseMID_1442ad650(
                    R.WindUpdatingBuildingSMActor, R.IntenseWindMaterialInstances[Slot]);
            if (!MildMID && !D.IntenseStateMID)
                E.LogBothMIDNull(R.WindUpdatingBuildingSMActor, Slot);
            D.MID = E.GetOrCreateMID_141c0a630(
                R.WindUpdatingBuildingSMActor, Mesh, Slot, EMIDCall::SecondCall);
            for (int32 I = 0; I < M.WindOffsetMaterialParameterNames.Num(); ++I)
            {
                const FName Offset = M.WindOffsetMaterialParameterNames[I];
                const FName Speed = M.WindSpeedMaterialParameterNames[I];
                // Native compares FName comparison index only for these two names.
                if ((I == 4 || I == 5) &&
                    (Offset.GetComparisonIndex() == FName(TEXT("Wind_Panner_Offset_1x_CodeReferenced")).GetComparisonIndex() ||
                     Offset.GetComparisonIndex() == FName(TEXT("Wind_Panner_Offset_2x_CodeReferenced")).GetComparisonIndex())) continue;
                if (!(E.HasScalar_141dccf40(M, D.MID, Offset) ||
                      E.HasScalar_141dccf40(M, D.MID, Speed) ||
                      E.HasScalar_141dccf40(M, MildMID, Offset) ||
                      E.HasScalar_141dccf40(M, MildMID, Speed) ||
                      E.HasScalar_141dccf40(M, D.IntenseStateMID, Offset) ||
                      E.HasScalar_141dccf40(M, D.IntenseStateMID, Speed))) continue;
                const uint32 Bit = 1u << (static_cast<uint32>(I) & 31u);
                D.MaterialParameterPairIndices = static_cast<int32>(static_cast<uint32>(D.MaterialParameterPairIndices) | Bit);
                if ((static_cast<uint32>(R.MaterialParameterPairIndices) & Bit) == 0)
                {
                    if (I > 7) { E.LogUnsupportedPair(R.WindUpdatingBuildingSMActor); continue; }
                    FFortWindResponderMaterialVariablePairData Pair{};
                    Pair.MaterialVariableIndex = I;
                    R.PairedVariablesData.Add(Pair);
                    R.MaterialParameterPairIndices = static_cast<int32>(static_cast<uint32>(R.MaterialParameterPairIndices) | Bit);
                }
                FFortWindMaterialParameterPairID ID{};
                ID.PairIndex = I; // VARIABLE index, not compact PairedVariablesData index.
                ID.SpeedParameter.VariableName = Speed;
                ID.SpeedParameter.VariableIndex = INDEX_NONE;
                ID.OffsetParameter.VariableName = Offset;
                ID.OffsetParameter.VariableIndex = INDEX_NONE;
                E.InitializePairScalar_144333900(D.MID, Speed, ID.SpeedParameter.VariableIndex);
                E.InitializePairScalar_144333900(D.MID, Offset, ID.OffsetParameter.VariableIndex);
                D.ParametersToSet.Add(ID);
            }
            static_assert(sizeof(FBox) == 0x1c, "Requires native 28-byte FBox");
            FBox Bounds;
            FMemory::Memcpy(&Bounds, R.NativeBounds_0x68, sizeof(Bounds));
            E.InitializeWindVector_1443339a0(D.MID, M.WindVectorParameterName,
                FLinearColor(1.f, 0.f, 0.f, (Bounds.Min.Z + Bounds.Max.Z) * .5f), D.WindVectorParameterIndex);
            UMaterialInterface* Base = E.BaseMaterial_141b3a3e0(MildMID);
            if (Base == E.BaseMaterial_141b3a3e0(D.IntenseStateMID))
            {
                for (FName Name : M.WindMaterialScalarParameterNamesToInterpolate)
                {
                    float From = 0.f, To = 0.f;
                    const bool HaveFrom = E.FindScalarOverrideInMIChain(MildMID, Name, From);
                    const bool HaveTo = E.FindScalarOverrideInMIChain(D.IntenseStateMID, Name, To);
                    if (!HaveFrom && !HaveTo) continue;
                    if (!HaveFrom) From = E.BaseScalar_3e0(Base, Name);
                    if (!HaveTo) To = E.BaseScalar_3e0(Base, Name);
                    if (From == To) continue;
                    FWindScalarMaterialInterpolationData Scalar{};
                    Scalar.MaterialParameterName = Name;
                    Scalar.MaterialParameterIndex = INDEX_NONE;
                    Scalar.LerpFromValue = From; Scalar.LerpToValue = To;
                    E.InitializeInterpolatedScalar_144333900(D.MID, Name, From, To, Scalar.MaterialParameterIndex);
                    D.ScalarInterpolationData.Add(Scalar);
                }
                for (FName Name : M.WindMaterialVectorParameterNamesToInterpolate)
                {
                    FLinearColor From(0,0,0,0), To(0,0,0,0);
                    const bool HaveFrom = E.FindVectorOverrideInMIChain(MildMID, Name, From);
                    const bool HaveTo = E.FindVectorOverrideInMIChain(D.IntenseStateMID, Name, To);
                    if (!HaveFrom && !HaveTo) continue;
                    if (!HaveFrom) From = E.BaseVector_3f8(Base, Name);
                    if (!HaveTo) To = E.BaseVector_3f8(Base, Name);
                    if (EqualComponents(From, To)) continue;
                    FWindVectorMaterialInterpolationData Vector{};
                    Vector.MaterialParameterName = Name;
                    Vector.MaterialParameterIndex = INDEX_NONE;
                    Vector.LerpFromValue = From; Vector.LerpToValue = To;
                    E.InitializeInterpolatedVector_1443339a0(D.MID, Name, From, To, Vector.MaterialParameterIndex);
                    D.VectorInterpolationData.Add(Vector);
                }
            }
        }
        R.MaterialsData.Add(D); // Deep copy, including an empty record for missing mild.
    }
    // Capture snapshots pair count here; no reset of existing arrays/masks occurs.
    const int32 NumPairs = R.PairedVariablesData.Num();
    for (int32 P = 0; P < NumPairs; ++P)
    {
        auto& Pair = R.PairedVariablesData[P];
        const int32 I = Pair.MaterialVariableIndex;
        Pair.SpeedVariableName = M.WindSpeedMaterialParameterNames.IsValidIndex(I)
            ? M.WindSpeedMaterialParameterNames[I] : NAME_None;
        Pair.TimeOffsetVariableName = M.WindOffsetMaterialParameterNames.IsValidIndex(I)
            ? M.WindOffsetMaterialParameterNames[I] : NAME_None;
        UCurveLinearColor* Curve = I < 4 ? R.WindSpeedCurve : R.WindPannerSpeedCurve;
        Pair.PreviousSpeed = Channel(Curve->GetLinearColorValue(0.f), I);
        Pair.PreviousOffset = 0.f;
        // Original visits first NumSlots entries, even when MaterialsData existed
        // before this call. Preserve this oddity; setup assumes fresh responder.
        for (int32 Slot = 0; Slot < NumSlots; ++Slot)
        {
            auto& D = R.MaterialsData[Slot];
            if (!D.MID || ((static_cast<uint32>(D.MaterialParameterPairIndices) >> (static_cast<uint32>(I) & 31u)) & 1u) == 0) continue;
            for (const auto& ID : D.ParametersToSet)
                if (ID.PairIndex == I)
                {
                    E.InitializePairWrites_14433ac70(D.MID, ID.SpeedParameter.VariableIndex,
                        ID.OffsetParameter.VariableIndex, Pair.PreviousSpeed, Pair.PreviousOffset);
                    break;
                }
        }
    }
    R.bHasSetupAnimatingMaterials = true;
}
} // namespace FortWindMaterialSetupDraft
