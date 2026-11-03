// CONFIDENCE: high; native writes the named scalar on IconMID when present.
{
    static const FName ParameterName(TEXT("TimerDuration"));
    if (IconMID)
    {
        IconMID->SetScalarParameterValue(ParameterName, Duration);
    }
}
