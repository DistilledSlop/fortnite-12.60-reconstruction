// CONFIDENCE: high; native writes the named scalar on IconMID when present.
{
    static const FName ParameterName(TEXT("Camera Push"));
    if (IconMID)
    {
        IconMID->SetScalarParameterValue(ParameterName, Offset);
    }
}
