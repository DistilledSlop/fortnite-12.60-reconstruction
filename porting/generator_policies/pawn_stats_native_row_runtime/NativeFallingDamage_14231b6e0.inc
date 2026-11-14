// Pawn context is passed by all recovered callers and unused by this base row.
if (FallingDamageTable == nullptr || FallingDamageTableRow.IsNone())
{
    UE_LOG(LogFort, Warning, TEXT("No falling damage table and/or table row name specified."));
    return 0.0f;
}
static const FString Context(TEXT("FallingDamageTable Lookup"));
const FRealCurve* Curve = FallingDamageTable->FindCurve(FallingDamageTableRow, Context, true);
if (Curve != nullptr)
{
    return Curve->Eval(Value, 0.0f);
}
UE_LOG(LogFort, Warning, TEXT("Falling curve not found in falling damage table: %s"), *FallingDamageTableRow.ToString());
return 0.0f;
