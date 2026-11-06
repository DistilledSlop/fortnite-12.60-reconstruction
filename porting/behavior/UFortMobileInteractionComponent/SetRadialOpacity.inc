// CONFIDENCE: high; native writes the named scalar on IconMID when present.
{
    static const FName ParameterName(TEXT("TimerOpacity"));
    if (IconMID)
    {
        IconMID->SetScalarParameterValue(ParameterName, Opacity);
    }
}
