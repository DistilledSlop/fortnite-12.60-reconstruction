// CONFIDENCE: high; native writes the named scalar on IconMID when present.
{
    static const FName ParameterName(TEXT("TimerStartTime"));
    if (IconMID)
    {
        IconMID->SetScalarParameterValue(ParameterName, StartTime);
    }
}
