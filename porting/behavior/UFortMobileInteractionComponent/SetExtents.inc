// CONFIDENCE: high; native converts XYZ to a color with alpha one and writes it to IconMID.
{
    static const FName ParameterName(TEXT("Interact Extents"));
    if (IconMID)
    {
        IconMID->SetVectorParameterValue(ParameterName, FLinearColor(Extents.X, Extents.Y, Extents.Z, 1.0f));
    }
}
