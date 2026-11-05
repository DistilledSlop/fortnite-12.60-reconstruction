// CONFIDENCE: HIGH
while (Instance)
{
    for (FScalarParameterValue& Value : Instance->ScalarParameterValues)
    {
        if (Value.ParameterInfo.Name == Name)
            return &Value;
    }
    Instance = Cast<UMaterialInstance>(Instance->Parent);
}
return nullptr;
