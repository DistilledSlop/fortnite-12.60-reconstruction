// CONFIDENCE: high; native writes the unchanged color vector to IconMID when present.
{
    static const FName ParameterName(TEXT("Color"));
    if (IconMID)
    {
        IconMID->SetVectorParameterValue(ParameterName, InColor);
    }
}
