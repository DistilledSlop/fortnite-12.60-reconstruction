// CONFIDENCE: HIGH (source recovery; build/runtime parity pending)
// Body for void AFortWindManager::NativeSetupAnimatingMaterials(FFortWindResponder& Responder).
// Local public base-value getters use newer caches than the captured Legacy traversal.
AFortWindManager& M = *this;
auto& R = Responder;

const auto FindScalarOverride = [](UMaterialInstance* Instance, FName Name, float& Out) -> bool
{
    if (const FScalarParameterValue* Value = AFortWindManager::NativeFindWindScalarOverride(Instance, Name))
    {
        Out = Value->ParameterValue;
        return true;
    }
    return false;
};
const auto FindVectorOverride = [](UMaterialInstance* Instance, FName Name, FLinearColor& Out) -> bool
{
    while (Instance)
    {
        for (const FVectorParameterValue& Value : Instance->VectorParameterValues)
            if (Value.ParameterInfo.Name == Name)
            {
                Out = Value.ParameterValue;
                return true;
            }
        Instance = Cast<UMaterialInstance>(Instance->Parent);
    }
    return false;
};
const auto Channel = [](const FLinearColor& V, int32 I) -> float
{
    switch (I) {
    case 0: case 4: return V.R;
    case 1: case 5: return V.G;
    case 2: case 6: return V.B;
    case 3: case 7: return V.A;
    default: return 0.0f;
    }
};
const auto EqualComponents = [](const FLinearColor& A, const FLinearColor& B) -> bool
{
    return A.R == B.R && A.G == B.G && A.B == B.B && A.A == B.A;
};
UMeshComponent* Mesh = R.WindUpdatingBuildingSMActor->StaticMeshComponent;
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
    if (!Mild)
    {
        UE_LOG(LogFortWind, Warning, TEXT("Failed to find mild wind material for wind responder %s, mesh component %s, material index %d."),
            *R.WindUpdatingBuildingSMActor->GetName(), *Mesh->GetName(), Slot);
    }
    else
    {
        UMaterialInstanceDynamic* MildMID = R.WindUpdatingBuildingSMActor->NativeGetOrCreateMID(Mesh, Slot);
        if (!MildMID)
        {
            UE_LOG(LogFortWind, Error, TEXT("GetOrCreateMID for wind responder %s (material index %d) failed!"),
                *R.WindUpdatingBuildingSMActor->GetName(), Slot);
        }
        if (Slot < R.IntenseWindMaterialInstances.Num())
            D.IntenseStateMID = UKismetMaterialLibrary::CreateDynamicMaterialInstance(
                R.WindUpdatingBuildingSMActor, R.IntenseWindMaterialInstances[Slot], NAME_None, EMIDCreationFlags::None);
        if (!MildMID && !D.IntenseStateMID)
            UE_LOG(LogFortWind, Error, TEXT("Mild and Intense MIDs are both NULL (Material Index %d, WR: %s).  Cannot add material data for this part of the wind updater."),
                Slot, *R.WindUpdatingBuildingSMActor->GetName());
        D.MID = R.WindUpdatingBuildingSMActor->NativeGetOrCreateMID(Mesh, Slot);
        for (int32 I = 0; I < M.WindOffsetMaterialParameterNames.Num(); ++I)
        {
            const FName Offset = M.WindOffsetMaterialParameterNames[I];
            const FName Speed = M.WindSpeedMaterialParameterNames[I];
            // Native compares FName comparison index only for these two names.
            if ((I == 4 || I == 5) &&
                (Offset.GetComparisonIndex() == FName(TEXT("Wind_Panner_Offset_1x_CodeReferenced")).GetComparisonIndex() ||
                 Offset.GetComparisonIndex() == FName(TEXT("Wind_Panner_Offset_2x_CodeReferenced")).GetComparisonIndex())) continue;
            if (!(AFortWindManager::NativeFindWindScalarOverride(D.MID, Offset) ||
                  AFortWindManager::NativeFindWindScalarOverride(D.MID, Speed) ||
                  AFortWindManager::NativeFindWindScalarOverride(MildMID, Offset) ||
                  AFortWindManager::NativeFindWindScalarOverride(MildMID, Speed) ||
                  AFortWindManager::NativeFindWindScalarOverride(D.IntenseStateMID, Offset) ||
                  AFortWindManager::NativeFindWindScalarOverride(D.IntenseStateMID, Speed))) continue;
            const uint32 Bit = 1u << (static_cast<uint32>(I) & 31u);
            D.MaterialParameterPairIndices = static_cast<int32>(static_cast<uint32>(D.MaterialParameterPairIndices) | Bit);
            if ((static_cast<uint32>(R.MaterialParameterPairIndices) & Bit) == 0)
            {
                if (I > 7)
            {
                UE_LOG(LogFortWind, Error, TEXT("WindResponder (%s) has more than 8 material variable pairs!  These aren't currently supported, so extra parameter pairs won't be handled at all."),
                    *R.WindUpdatingBuildingSMActor->GetName());
                continue;
            }
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
            D.MID->InitializeScalarParameterAndGetIndex(Speed, 0.f, ID.SpeedParameter.VariableIndex);
            D.MID->InitializeScalarParameterAndGetIndex(Offset, 0.f, ID.OffsetParameter.VariableIndex);
            D.ParametersToSet.Add(ID);
        }
        static_assert(sizeof(FBox) == 0x1c, "Requires native 28-byte FBox");
        FBox Bounds;
        FMemory::Memcpy(&Bounds, R.NativeBounds_0x68, sizeof(Bounds));
        D.MID->InitializeVectorParameterAndGetIndex(M.WindVectorParameterName,
            FLinearColor(1.f, 0.f, 0.f, (Bounds.Min.Z + Bounds.Max.Z) * .5f), D.WindVectorParameterIndex);
        UMaterial* Base = MildMID->GetBaseMaterial();
        if (Base == D.IntenseStateMID->GetBaseMaterial())
        {
            for (FName Name : M.WindMaterialScalarParameterNamesToInterpolate)
            {
                float From = 0.f, To = 0.f;
                const bool HaveFrom = FindScalarOverride(MildMID, Name, From);
                const bool HaveTo = FindScalarOverride(D.IntenseStateMID, Name, To);
                if (!HaveFrom && !HaveTo) continue;
                // Outputs start at zero; native ignores getter bool. Global
                // info/false override-only match captured virtual0x3e0.
                // Local public API uses a newer cache than original Legacy traversal.
                if (!HaveFrom) Base->GetScalarParameterValue(FHashedMaterialParameterInfo(Name), From, false);
                if (!HaveTo) Base->GetScalarParameterValue(FHashedMaterialParameterInfo(Name), To, false);
                if (From == To) continue;
                FWindScalarMaterialInterpolationData Scalar{};
                Scalar.MaterialParameterName = Name;
                Scalar.MaterialParameterIndex = INDEX_NONE;
                Scalar.LerpFromValue = From; Scalar.LerpToValue = To;
                D.MID->InitializeScalarParameterAndGetIndex(Name, From, Scalar.MaterialParameterIndex);
                D.ScalarInterpolationData.Add(Scalar);
            }
            for (FName Name : M.WindMaterialVectorParameterNamesToInterpolate)
            {
                FLinearColor From(0,0,0,0), To(0,0,0,0);
                const bool HaveFrom = FindVectorOverride(MildMID, Name, From);
                const bool HaveTo = FindVectorOverride(D.IntenseStateMID, Name, To);
                if (!HaveFrom && !HaveTo) continue;
                // Zero outputs, global info and false override-only; ignore
                // bool, matching captured virtual0x3f8 (Legacy traversal).
                if (!HaveFrom) Base->GetVectorParameterValue(FHashedMaterialParameterInfo(Name), From, false);
                if (!HaveTo) Base->GetVectorParameterValue(FHashedMaterialParameterInfo(Name), To, false);
                if (EqualComponents(From, To)) continue;
                FWindVectorMaterialInterpolationData Vector{};
                Vector.MaterialParameterName = Name;
                Vector.MaterialParameterIndex = INDEX_NONE;
                Vector.LerpFromValue = From; Vector.LerpToValue = To;
                D.MID->InitializeVectorParameterAndGetIndex(Name, From, Vector.MaterialParameterIndex);
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
                D.MID->SetScalarParameterByIndex(ID.SpeedParameter.VariableIndex, Pair.PreviousSpeed);
                D.MID->SetScalarParameterByIndex(ID.OffsetParameter.VariableIndex, 0.f);
                break;
            }
    }
}
R.bHasSetupAnimatingMaterials = true;
