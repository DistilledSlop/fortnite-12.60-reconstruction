if (VehicleEjectDamageTable)
{
    static const FString Context(TEXT("VehicleEjectDamageTable Lookup"));
    static const FName Row(TEXT("Player_VehicleEject"));
    const FRealCurve* Curve = VehicleEjectDamageTable->FindCurve(Row, Context, true);
    if (Curve)
        return Curve->Eval(Value, 0.0f);
}
return 0.0f;
