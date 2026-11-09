// Original address: 0x141ee94b0; decompile source: porting/generated/native-cap-direct/0x141ee94b0.c.
// CONFIDENCE: HIGH
// Exec 0x142968380 forwards to the native body; copy each present scalar override.
if (SourceMID && TargetMID)
{
    float Value = 0.0f;
    if (SourceMID->GetScalarParameterValue(::FHashedMaterialParameterInfo(FName(TEXT("Hide Element 01"))), Value, true))
        TargetMID->SetScalarParameterValue(FName(TEXT("Hide Element 01")), Value);
    if (SourceMID->GetScalarParameterValue(::FHashedMaterialParameterInfo(FName(TEXT("Hide Element 02"))), Value, true))
        TargetMID->SetScalarParameterValue(FName(TEXT("Hide Element 02")), Value);
    if (SourceMID->GetScalarParameterValue(::FHashedMaterialParameterInfo(FName(TEXT("Hide Element 03"))), Value, true))
        TargetMID->SetScalarParameterValue(FName(TEXT("Hide Element 03")), Value);
    if (SourceMID->GetScalarParameterValue(::FHashedMaterialParameterInfo(FName(TEXT("Hide Element 04"))), Value, true))
        TargetMID->SetScalarParameterValue(FName(TEXT("Hide Element 04")), Value);
    if (SourceMID->GetScalarParameterValue(::FHashedMaterialParameterInfo(FName(TEXT("Hide Element 05"))), Value, true))
        TargetMID->SetScalarParameterValue(FName(TEXT("Hide Element 05")), Value);
    if (SourceMID->GetScalarParameterValue(::FHashedMaterialParameterInfo(FName(TEXT("Hide Element 06"))), Value, true))
        TargetMID->SetScalarParameterValue(FName(TEXT("Hide Element 06")), Value);
    if (SourceMID->GetScalarParameterValue(::FHashedMaterialParameterInfo(FName(TEXT("Hide Element 07"))), Value, true))
        TargetMID->SetScalarParameterValue(FName(TEXT("Hide Element 07")), Value);
    if (SourceMID->GetScalarParameterValue(::FHashedMaterialParameterInfo(FName(TEXT("Hide Element 08"))), Value, true))
        TargetMID->SetScalarParameterValue(FName(TEXT("Hide Element 08")), Value);
    if (SourceMID->GetScalarParameterValue(::FHashedMaterialParameterInfo(FName(TEXT("Hide Element 09"))), Value, true))
        TargetMID->SetScalarParameterValue(FName(TEXT("Hide Element 09")), Value);
    if (SourceMID->GetScalarParameterValue(::FHashedMaterialParameterInfo(FName(TEXT("Hide Element 10"))), Value, true))
        TargetMID->SetScalarParameterValue(FName(TEXT("Hide Element 10")), Value);
}
