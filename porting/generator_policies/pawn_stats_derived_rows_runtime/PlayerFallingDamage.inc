const UFortAbilitySystemComponent* AbilitySystem = Cast<UFortAbilitySystemComponent>(Pawn->GetAbilitySystemComponent());
bool bPersonalVehicle = false;
if (AbilitySystem)
{
    const FGameplayTag& PersonalVehicleTag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x6b0);
    bPersonalVehicle = AbilitySystem->HasMatchingGameplayTag(PersonalVehicleTag);
}
const FName Row = bPersonalVehicle ? PersonalVehicleFallingDamageTableRow : FallingDamageTableRow;
if (FallingDamageTable == nullptr || Row.IsNone())
{
    if (bPersonalVehicle)
    {
        UE_LOG(LogFort, Warning, TEXT("No falling damage table and/or personal vehicle table row name specified."));
    }
    else
    {
        UE_LOG(LogFort, Warning, TEXT("No falling damage table and/or table row name specified."));
    }
    return 0.0f;
}
static const FString Context(TEXT("FallingDamageTable Lookup"));
const FRealCurve* Curve = FallingDamageTable->FindCurve(Row, Context, true);
if (Curve)
    return Curve->Eval(Value, 0.0f);
if (bPersonalVehicle)
{
    UE_LOG(LogFort, Warning, TEXT("Falling curve for the personal vehicle not found in falling damage table: %s"), *Row.ToString());
}
else
{
    UE_LOG(LogFort, Warning, TEXT("Falling curve not found in falling damage table: %s"), *Row.ToString());
}
return 0.0f;
