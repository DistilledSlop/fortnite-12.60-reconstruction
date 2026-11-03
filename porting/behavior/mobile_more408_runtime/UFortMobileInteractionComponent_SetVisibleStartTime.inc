// CONFIDENCE: high; native writes the component world time to the material when both exist.
{
    static const FName ParameterName(TEXT("PulseStartTime"));
    if (IconMID)
    {
        if (const UWorld* World = GetWorld())
        {
            IconMID->SetScalarParameterValue(ParameterName, World->GetTimeSeconds());
        }
    }
}
