// CONFIDENCE: HIGH
    D.SectionCurrentBlendTime = D.SectionBlendTime;
    D.SectionCurrentMagnitude = D.PreviousMagnitude;

    float OuterRadius;
    if (!D.bRippleOutward)
    {
        OuterRadius = D.InnerSectionRadius;
        D.OuterSectionRadius = OuterRadius;
        if (OuterRadius == 0.0f)
            return false;
        D.InnerSectionRadius = OuterRadius - D.SectionWidth;
        if (D.InnerSectionRadius < 0.0f)
            D.InnerSectionRadius = 0.0f;
    }
    else
    {
        OuterRadius = D.OuterSectionRadius;
        const float MaximumRadius = D.MaximumRadius;
        D.InnerSectionRadius = OuterRadius;
        if (OuterRadius == MaximumRadius)
            return false;
        OuterRadius = OuterRadius + D.SectionWidth;
        D.OuterSectionRadius = OuterRadius;
        if (MaximumRadius < OuterRadius)
        {
            D.OuterSectionRadius = MaximumRadius;
            OuterRadius = MaximumRadius;
        }
    }

    FBox OuterBox;
    OuterBox.Min = FVector(D.DeltaCenter.X - OuterRadius,
                           D.DeltaCenter.Y - OuterRadius,
                           D.WindImpulseBounds.Min.Z);
    OuterBox.Max = FVector(D.DeltaCenter.X + OuterRadius,
                           D.DeltaCenter.Y + OuterRadius,
                           D.WindImpulseBounds.Max.Z);
    OuterBox.IsValid = 1;
    D.OuterWorldBounds = OuterBox;
    D.OuterWorldBounds = NativeClipWindImpulseBounds(D.OuterWorldBounds, D.WindImpulseBounds);

    const float InnerBoxRadius = D.InnerSectionRadius * 0.70710677f;
    FBox InnerBox;
    InnerBox.Min = FVector(D.DeltaCenter.X - InnerBoxRadius,
                           D.DeltaCenter.Y - InnerBoxRadius,
                           D.WindImpulseBounds.Min.Z);
    InnerBox.Max = FVector(D.DeltaCenter.X + InnerBoxRadius,
                           D.DeltaCenter.Y + InnerBoxRadius,
                           D.WindImpulseBounds.Max.Z);
    InnerBox.IsValid = 1;
    D.InnerWorldBounds = InnerBox;
    D.InnerWorldBounds = NativeClipWindImpulseBounds(D.InnerWorldBounds, D.WindImpulseBounds);
    return true;
