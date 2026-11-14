// Deliberately no table-null guard: the original unconditionally calls FindCurve.
static const FString Context(TEXT("FallingDamageTable Lookup"));
const FRealCurve* Curve = FallingDamageTable->FindCurve(FallingDamageTableRow, Context, true);
float Result = 0.0f;
if (Curve != nullptr)
{
    for (FKeyHandle Key = Curve->GetFirstKeyHandle(); Key != FKeyHandle::Invalid(); Key = Curve->GetNextKey(Key))
    {
        if (Curve->GetKeyValue(Key) > 0.0f)
        {
            return Result;
        }
        Result = Curve->GetKeyTime(Key);
    }
}
return Result;
