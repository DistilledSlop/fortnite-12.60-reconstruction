// CONFIDENCE: HIGH (native source recovery; build/runtime validation pending).
// Native 0x141ddfad0.
// Body of AFortWindManager::NativeSampleWind.
// All three sampling paths use the same assembly predicate. Minimum NaNs reject;
// maximum NaNs pass. FBox::IsValid is not consulted. Decompiled conditional minima
// were an expression-recovery artifact (141ddfee0..ff1c / 141de009d..00d9).
const auto InCaptureBounds = [&](const FBox& B, const FVector& P)
{
    return P.X >= B.Min.X && !(P.X > B.Max.X)
        && P.Y >= B.Min.Y && !(P.Y > B.Max.Y)
        && P.Z >= B.Min.Z && !(P.Z > B.Max.Z);
};

const auto ZeroOrUnordered = [](float V) { return !(V < 0.0f) && !(V > 0.0f); };

// Requires ordinary scalar float evaluation, no contraction/reassociation. The
// second refinement reuses HalfSquared; it does not recompute S * 0.5f.
const auto RefinedReciprocalSqrt = [&](float S)
{
    float R = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(S)));
    const float HalfSquared = S * 0.5f;
    float Square = R * R;
    float Error = 0.5f - HalfSquared * Square;
    R = R + R * Error;
    Square = R * R;
    Error = 0.5f - HalfSquared * Square;
    R = R + R * Error;
    return R;
};

const auto RadiusContribution = [&](const FFortWindImpulseRadius& R, const FVector& P)
{
    if (!(R.CurrentRadius > 0.0f)) return FVector::ZeroVector;
    const FVector V = P - R.Location;
    const float Distance = FMath::Sqrt((V.X * V.X + V.Y * V.Y) + V.Z * V.Z);
    FVector Direction = FVector::ZeroVector;
    if (Distance > 1.e-8f)
    {
        const float Reciprocal = 1.0f / Distance;
        Direction = FVector(Reciprocal * V.X, Reciprocal * V.Y, Reciprocal * V.Z);
    }
    // COMISS/JB enters for unordered distance, too. NaN distance therefore
    // produces NaN magnitude times zero direction, not an early zero return.
    if (Distance >= R.CurrentRadius) return FVector::ZeroVector;
    const float Magnitude = R.CurrentMagnitude - (R.CurrentMagnitude / R.CurrentRadius) * Distance;
    return FVector(Magnitude * Direction.X, Magnitude * Direction.Y, Magnitude * Direction.Z);
};

const auto CylinderDirectionWithFalloff = [&](const FFortWindImpulseCylinder& C, const FVector& P,
    bool bDelta = false, bool bDistanceXY = false)
{
    if (!(C.OuterRadius > 0.0f)) return FVector::ZeroVector;
    const float DX = P.X - C.Location.X, DY = P.Y - C.Location.Y;
    const float XX = DX * DX, YY = DY * DY;
    FVector Direction = C.WindDirection;
    // UCOMISS/JNE sees unordered as equal. Preserve Z even on the radial
    // path: it can be -0 or NaN; native only replaces X and Y here.
    if (ZeroOrUnordered(Direction.X) && ZeroOrUnordered(Direction.Y) && ZeroOrUnordered(Direction.Z))
    {
        const float S = bDelta ? XX + YY : YY + XX;
        Direction.X = 0.0f;
        Direction.Y = 0.0f;
        if (S > 1.e-8f)
        {
            const float Reciprocal = RefinedReciprocalSqrt(S);
            Direction.X = Reciprocal * DX;
            Direction.Y = Reciprocal * DY;
        }
    }
    // Regular cylinders and four delta branches sum YY+XX; outer/inward
    // delta sums XX+YY. Keep this distinction for NaN operand propagation.
    const float Distance = FMath::Sqrt(bDistanceXY ? XX + YY : YY + XX);
    if (Distance >= C.OuterRadius) return FVector::ZeroVector;
    if (Distance > C.InnerRadius)
    {
        const float Falloff = 1.0f - (Distance - C.InnerRadius) / (C.OuterRadius - C.InnerRadius);
        Direction = FVector(Falloff * Direction.X, Falloff * Direction.Y, Direction.Z * Falloff);
    }
    return Direction;
};

const auto Sample = [&](const FVector& P,
    bool bHaveDeltaCylinder, const FFortWindImpulseCylinder& DeltaCylinder)
{
    FVector Wind = NativeWindVector_0x430;
    for (const FFortWindImpulseRadius& R : WindImpulses)
        if (InCaptureBounds(R.WorldBounds, P)) Wind += RadiusContribution(R, P);
    for (const FFortWindImpulseCylinder& C : WindImpulseCylinders)
        // UCOMISS/JE skips zero and unordered magnitude.
        if (!C.bIsChanging && !ZeroOrUnordered(C.Magnitude) && InCaptureBounds(C.WorldBounds, P))
            Wind += CylinderDirectionWithFalloff(C, P) * C.Magnitude;

    if (bHaveDeltaCylinder && WindImpulseCylinderDeltas.Num() > 0)
    {
        const FFortWindImpulseCylinderDelta& D = WindImpulseCylinderDeltas[0];
        // COMISS/JB rejects unordered minimum comparisons; COMISS/JA accepts
        // unordered maximum comparisons. Preserve this asymmetry.
        const FBox& B = D.WindImpulseBounds;
        if (P.X >= B.Min.X && !(P.X > B.Max.X)
            && P.Y >= B.Min.Y && !(P.Y > B.Max.Y)
            && P.Z >= B.Min.Z && !(P.Z > B.Max.Z))
        {
            const float DX = P.X - D.DeltaCenter.X, DY = P.Y - D.DeltaCenter.Y;
            const float Distance = FMath::Sqrt(DY * DY + DX * DX);
            float Magnitude;
            // JAE selects the next test only for ordered Distance >= inner;
            // JBE selects the band for ordered <= outer or unordered.
            if (!(Distance >= D.InnerSectionRadius))
                Magnitude = D.bRippleOutward ? D.DesiredMagnitude : D.PreviousMagnitude;
            else if (!(Distance > D.OuterSectionRadius))
                Magnitude = D.SectionCurrentMagnitude;
            else
                Magnitude = D.bRippleOutward ? D.PreviousMagnitude : D.DesiredMagnitude;
            const bool bOuterInward = Distance >= D.InnerSectionRadius
                && Distance > D.OuterSectionRadius && !D.bRippleOutward;
            Wind += CylinderDirectionWithFalloff(DeltaCylinder, P, true, bOuterInward) * Magnitude;
        }
    }
    return Wind;
};

const auto ResponderBounds = [&](const FFortWindResponder& R)
{
    FBox Bounds;
    static_assert(sizeof(Bounds) == sizeof(R.NativeBounds_0x68), "Native bounds contract");
    FMemory::Memcpy(&Bounds, R.NativeBounds_0x68, sizeof(Bounds));
    return Bounds;
};

const auto UpdateMaterials = [&](FFortWindResponder& R,
    float Speed, const FVector& Direction, float Time, float DeltaSeconds)
{
    R.WindSpeed = Speed;
    if (R.MildWindMaterialInstances.Num() <= 0) return;
    if (!R.bHasSetupAnimatingMaterials) NativeSetupAnimatingMaterials(R);
    for (FFortWindResponderMaterialVariablePairData& Pair : R.PairedVariablesData)
    {
        UCurveLinearColor* Curve = Pair.MaterialVariableIndex < 4 ? R.WindSpeedCurve : R.WindPannerSpeedCurve;
        const FLinearColor Value = Curve->GetLinearColorValue(R.WindSpeed);
        float NewSpeed = 0.0f;
        switch (Pair.MaterialVariableIndex)
        {
        case 0: case 4: NewSpeed = Value.R; break;
        case 1: case 5: NewSpeed = Value.G; break;
        case 2: case 6: NewSpeed = Value.B; break;
        case 3: case 7: NewSpeed = Value.A; break;
        }
        const float PreviousTime = (Time - DeltaSeconds) * Pair.PreviousSpeed + Pair.PreviousOffset;
        Pair.PreviousSpeed = NewSpeed;
        Pair.DeltaTimeModifiedByMaterialSpeed = NewSpeed * DeltaSeconds;
        Pair.MaterialsPreviousTime = PreviousTime;
        Pair.PreviousOffset = (NewSpeed * DeltaSeconds + PreviousTime) - NewSpeed * Time;
    }
    for (FFortWindMaterialData& Data : R.MaterialsData)
    {
        if (!Data.MID) continue;
        for (const FWindScalarMaterialInterpolationData& Scalar : Data.ScalarInterpolationData)
            Data.MID->SetScalarParameterByIndex(Scalar.MaterialParameterIndex,
                (Scalar.LerpToValue - Scalar.LerpFromValue) * R.WindSpeed + Scalar.LerpFromValue);
        for (const FWindVectorMaterialInterpolationData& Vector : Data.VectorInterpolationData)
            Data.MID->SetVectorParameterByIndex(Vector.MaterialParameterIndex,
                (Vector.LerpToValue - Vector.LerpFromValue) * R.WindSpeed + Vector.LerpFromValue);
        for (const FFortWindMaterialParameterPairID& ID : Data.ParametersToSet)
        {
            // Machine code141de0e45..0e7d confirms direct compact-array indexing,
            // even though setup stores material variable IDs here. Preserve shipping behavior.
            if (!R.PairedVariablesData.IsValidIndex(ID.PairIndex)) continue;
            const auto& Pair = R.PairedVariablesData[ID.PairIndex];
            Data.MID->SetScalarParameterByIndex(ID.SpeedParameter.VariableIndex, Pair.PreviousSpeed);
            Data.MID->SetScalarParameterByIndex(ID.OffsetParameter.VariableIndex, Pair.PreviousOffset);
        }
        // Original skips this write at zero speed, leaving the previous vector parameter.
        if (R.WindSpeed != 0.0f)
        {
            const FBox Bounds = ResponderBounds(R);
            Data.MID->SetVectorParameterByIndex(Data.WindVectorParameterIndex,
                FLinearColor(Direction.X * Speed, Direction.Y * Speed, Direction.Z * Speed,
                    (Bounds.Min.Z + Bounds.Max.Z) * 0.5f));
        }
    }
};

const auto ReplaceFirst = [&](TArray<int32>& Array, int32 From, int32 To)
{
    const int32 Index = Array.Find(From);
    if (Index != INDEX_NONE) Array[Index] = To;
};

const auto ToBox2D = [](const FBox& Bounds)
{
    return FBox2D(FVector2D(Bounds.Min.X, Bounds.Min.Y), FVector2D(Bounds.Max.X, Bounds.Max.Y));
};

const auto RemoveResponder = [&](int32 Removed)
{
    NativeWindResponderTree_0x2e0->Remove(Removed, ToBox2D(ResponderBounds(WindResponders[Removed])));
    const int32 Last = WindResponders.Num() - 1;
    if (Last > 0 && Last != Removed)
    {
        const FBox LastBounds = ResponderBounds(WindResponders[Last]);
        NativeWindResponderTree_0x2e0->Remove(Last, ToBox2D(LastBounds));
        NativeWindResponderTree_0x2e0->Insert(Removed, ToBox2D(LastBounds), nullptr);
        ReplaceFirst(NearbyCacheIndicesForUpdates, Last, Removed);
        ReplaceFirst(NearbyCacheIndicesForAudio, Last, Removed);
    }
    WindResponders.RemoveAtSwap(Removed, 1, true); // Nested arrays must be destroyed.
    NearbyCacheIndicesForUpdates.RemoveAtSwap(NextNearbyIndexToUpdate, 1, true);
    const int32 AudioIndex = NearbyCacheIndicesForAudio.Find(Removed);
    if (AudioIndex != INDEX_NONE) NearbyCacheIndicesForAudio.RemoveAtSwap(AudioIndex, 1, true);
    --NextNearbyIndexToUpdate;
};

const auto Smooth = [&](float T)
{
    const float T2 = T * T;
    return T2 * 3.0f - (T2 + T2) * T;
};

const auto UpdateRadiusImpulses = [&](float DeltaSeconds)
{
    for (int32 Index = WindImpulses.Num() - 1; Index >= 0; --Index)
    {
        FFortWindImpulseRadius& R = WindImpulses[Index];
        const bool bRadiusChanged = R.Radius != R.CurrentRadius;
        if (!bRadiusChanged && R.Magnitude == R.CurrentMagnitude) continue;
        // Source keeps this local at desired magnitude in completion branches.
        float MagnitudeForRemoval = R.Magnitude;
        if (R.BlendTime > 0.0f)
        {
            R.CurrentBlendTime -= DeltaSeconds;
            if (R.CurrentBlendTime > 0.0f)
            {
                const float Alpha = Smooth(1.0f - R.CurrentBlendTime / R.BlendTime);
                MagnitudeForRemoval = (R.Magnitude - R.PreviousMagnitude) * Alpha + R.PreviousMagnitude;
                R.CurrentMagnitude = MagnitudeForRemoval;
                if (bRadiusChanged)
                {
                    R.CurrentRadius = (R.Radius - R.PreviousRadius) * Alpha + R.PreviousRadius;
                    const FVector Extent(R.CurrentRadius);
                    R.WorldBounds = FBox(R.Location - Extent, R.Location + Extent);
                }
            }
            else { R.CurrentMagnitude = R.Magnitude; R.CurrentRadius = R.Radius; }
        }
        else { R.CurrentMagnitude = R.Magnitude; R.CurrentRadius = R.Radius; }
        // Bounds are NOT recomputed in completion branches in this capture.
        if (R.Magnitude == 0.0f && MagnitudeForRemoval == 0.0f)
            WindImpulses.RemoveAtSwap(Index, 1, false);
    }
};

const auto UpdateDeltaImpulses = [&](float DeltaSeconds)
{
    TArray<FBox> SectionBoxes; // 16-byte local, freed on exit; contents unused by this function.
    if (WindImpulseCylinderDeltas.Num() > 0)
    {
        FFortWindImpulseCylinderDelta& D = WindImpulseCylinderDeltas[0];
        bool bSectionComplete = D.DesiredMagnitude == D.SectionCurrentMagnitude;
        if (!bSectionComplete)
        {
            if (D.SectionBlendTime > 0.0f)
            {
                D.SectionCurrentBlendTime -= DeltaSeconds;
                if (D.SectionCurrentBlendTime <= 0.0f)
                { D.SectionCurrentMagnitude = D.DesiredMagnitude; bSectionComplete = true; }
                else
                {
                    const float Target = (D.DesiredMagnitude - D.PreviousMagnitude)
                        * Smooth(1.0f - D.SectionCurrentBlendTime / D.SectionBlendTime) + D.PreviousMagnitude;
                    float Lower;
                    float Upper;
                    if (D.DesiredMagnitude - D.PreviousMagnitude < 0.0f)
                    { Lower = D.DesiredMagnitude; Upper = D.PreviousMagnitude; }
                    else
                    { Lower = D.PreviousMagnitude; Upper = D.DesiredMagnitude; }
                    float Result = Lower;
                    if (Lower <= Target)
                    {
                        Result = Target;
                        if (Upper <= Target) Result = Upper;
                    }
                    D.SectionCurrentMagnitude = Result;
                    bSectionComplete = D.SectionCurrentMagnitude == D.DesiredMagnitude;
                }
            }
            else { D.SectionCurrentMagnitude = D.DesiredMagnitude; bSectionComplete = true; }
            NativeBuildWindSectionBoxes(D, SectionBoxes);
        }
        if (bSectionComplete && !NativeAdvanceWindDelta(D))
        {
            const int32 UID = D.WindImpulseHandleToModify.UID;
            const float DesiredMagnitude = D.DesiredMagnitude;
            const int32 CylinderIndex = WindImpulseCylinders.IndexOfByPredicate(
                [UID](const FFortWindImpulseCylinder& C) { return C.Handle.UID == UID; });
            WindImpulseCylinderDeltas.RemoveAtSwap(0, 1, false);
            bool bSameCylinderStillFirst = false;
            if (CylinderIndex != INDEX_NONE)
            {
                FFortWindImpulseCylinder& C = WindImpulseCylinders[CylinderIndex];
                C.Magnitude = DesiredMagnitude;
                const int32 NextDeltaIndex = WindImpulseCylinderDeltas.IndexOfByPredicate(
                    [UID](const FFortWindImpulseCylinderDelta& Next) { return Next.WindImpulseHandleToModify.UID == UID; });
                bSameCylinderStillFirst = NextDeltaIndex == 0;
                C.bIsChanging = bSameCylinderStillFirst;
                if (!bSameCylinderStillFirst)
                {
                    OnWindImpulseCylinderDeltaComplete(C);
                    if (NextDeltaIndex != INDEX_NONE) C.bIsChangePending = true;
                }
                if (C.Magnitude == 0.0f && !C.bIsChanging && !C.bIsChangePending)
                {
                    OnWindImpulseCylinderDeath(C);
                    WindImpulseCylinders.RemoveAtSwap(CylinderIndex, 1, false);
                }
            }
            if (!bSameCylinderStillFirst) NativeStartQueuedWindDelta();
        }
    }
    // Source frees SectionBoxes before the final cylinder cleanup; release at same boundary.
    SectionBoxes.Empty();
    for (int32 Index = WindImpulseCylinders.Num() - 1; Index >= 0; --Index)
    {
        const auto& C = WindImpulseCylinders[Index];
        if (C.Magnitude == 0.0f && !C.bIsChanging && !C.bIsChangePending)
        {
            OnWindImpulseCylinderDeath(C);
            WindImpulseCylinders.RemoveAtSwap(Index, 1, false);
        }
    }
};


if (!NativeIsWindEnabled()) return;
if (GetNetMode() == NM_DedicatedServer)
{
    UE_LOG(LogFortWind, Log, TEXT("AFortWindManager::TickActor() called on dedicated server.  No need to tick on the dedicated server.  In fact, this actor should not EXIST on dedicated server!  Destroying it."));
    Destroy(false, true);
    return;
}
UWorld* World = GetWorld();
if (!World)
{
    UE_LOG(LogFortWind, Error, TEXT("AFortWindManager::TickActor() has nullptr MyWorld.  Cannot update wind impulses."));
    return;
}
const float WorldTime = World->GetTimeSeconds();
{
    DECLARE_CYCLE_STAT(TEXT("Fort Update Wind Responders"), STAT_Fort_UpdateWindResponders, STATGROUP_Game);
    SCOPE_CYCLE_COUNTER(STAT_Fort_UpdateWindResponders);
    // Even with no deltas this temporary consumes exactly one shared native handle UID.
    FFortWindImpulseCylinder DeltaCylinder{};
    bool bHaveDeltaCylinder = false;
    if (WindImpulseCylinderDeltas.Num() > 0)
    {
        bHaveDeltaCylinder = NativeGetWindImpulseCylinder(
            WindImpulseCylinderDeltas[0].WindImpulseHandleToModify, DeltaCylinder);
    }
    const int32 Count = bAllowWindImpulses
        ? FMath::Min(NearbyCacheIndicesForUpdates.Num(), NativeWindUpdateBudget_0x220)
        : NearbyCacheIndicesForUpdates.Num();
    for (int32 Iteration = 0; Iteration < Count; ++Iteration)
    {
        const int32 Cursor = NextNearbyIndexToUpdate;
        const int32 ResponderIndex = NearbyCacheIndicesForUpdates[Cursor];
        // Capture tests Cursor against responder count; do not silently substitute cached index.
        if (Cursor >= 0 && Cursor < WindResponders.Num())
        {
            FFortWindResponder& R = WindResponders[ResponderIndex];
            if (R.WindUpdatingBuildingSMActor && !R.WindUpdatingBuildingSMActor->IsPendingKill())
            {
                const FVector Wind = bAllowWindImpulses ? Sample(
                    R.WindUpdatingBuildingSMActor->GetActorLocation(), bHaveDeltaCylinder, DeltaCylinder)
                    : NativeWindVector_0x430;
                const float Length = FMath::Sqrt((Wind.X * Wind.X + Wind.Y * Wind.Y) + Wind.Z * Wind.Z);
                const float Reciprocal = Length > 1.e-8f ? 1.0f / Length : 0.0f;
                const FVector Direction = Length > 1.e-8f
                    ? FVector(Wind.X * Reciprocal, Wind.Y * Reciprocal, Wind.Z * Reciprocal)
                    : FVector::ZeroVector;
                // COMISS/JAE maps unordered length to +0; upper comparison
                // maps equality/+infinity to 1. Signed zero survives lower test.
                const float Speed = !(Length >= 0.0f) ? 0.0f : (Length < 1.0f ? Length : 1.0f);
                // Direction-only changes do not cause a material write.
                if (FMath::Abs(Speed - R.WindSpeed) > 1.e-8f)
                    UpdateMaterials(R, Speed, Direction, WorldTime, DeltaSeconds);
            }
            else RemoveResponder(ResponderIndex);
        }
        NextNearbyIndexToUpdate = NearbyCacheIndicesForUpdates.Num() < 1 ? 0
            : (NextNearbyIndexToUpdate + 1) % NearbyCacheIndicesForUpdates.Num();
    }
}
if (!bAllowWindImpulses) return;
{
    DECLARE_CYCLE_STAT(TEXT("Fort Update Wind Impulses"), STAT_Fort_UpdateWindImpulses, STATGROUP_Game);
    SCOPE_CYCLE_COUNTER(STAT_Fort_UpdateWindImpulses);
    UpdateRadiusImpulses(DeltaSeconds);
}
UpdateDeltaImpulses(DeltaSeconds);
