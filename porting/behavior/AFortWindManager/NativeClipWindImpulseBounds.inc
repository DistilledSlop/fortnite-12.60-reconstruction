// CONFIDENCE: HIGH
    if ((A.Min.X < B.Max.X || A.Min.X == B.Max.X) && B.Min.X <= A.Max.X &&
        (A.Min.Y < B.Max.Y || A.Min.Y == B.Max.Y) && B.Min.Y <= A.Max.Y &&
        (A.Min.Z < B.Max.Z || A.Min.Z == B.Max.Z) && B.Min.Z <= A.Max.Z)
    {
        const FVector Min(B.Min.X <= A.Min.X ? A.Min.X : B.Min.X,
                          B.Min.Y <= A.Min.Y ? A.Min.Y : B.Min.Y,
                          B.Min.Z <= A.Min.Z ? A.Min.Z : B.Min.Z);
        const FVector Max(B.Max.X <= A.Max.X ? B.Max.X : A.Max.X,
                          B.Max.Y <= A.Max.Y ? B.Max.Y : A.Max.Y,
                          B.Max.Z <= A.Max.Z ? B.Max.Z : A.Max.Z);
        return FBox(Min, Max);
    }
    return FBox(ForceInit);
