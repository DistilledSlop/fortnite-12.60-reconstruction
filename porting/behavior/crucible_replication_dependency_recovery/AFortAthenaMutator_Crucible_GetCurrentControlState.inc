// CONFIDENCE: HIGH; original capture and dependency evidence in evidence.json.
if (GetLocalRole() != ROLE_Authority && !NativeRecoveredBool_0x4f1 && GetWorld())
{
    if (APlayerController* FirstPC = GetWorld()->GetFirstPlayerController())
    {
        if (AFortPlayerControllerAthena* AthenaPC = Cast<AFortPlayerControllerAthena>(FirstPC))
        {
            if (AFortPlayerStateAthena* AthenaPS = Cast<AFortPlayerStateAthena>(AthenaPC->PlayerState))
            {
                // The original comparison folds ASCII only and compares complete null-terminated strings.
                const auto IsPlatform = [&AthenaPS](const TCHAR* Expected)
                {
                    const TCHAR* Actual = *AthenaPS->Platform;
                    while (*Actual && *Expected)
                    {
                        TCHAR C = *Actual++;
                        if (C >= TEXT('a') && C <= TEXT('z')) C -= TEXT('a') - TEXT('A');
                        if (C != *Expected++) return false;
                    }
                    return *Actual == 0 && *Expected == 0;
                };
                if (IsPlatform(TEXT("AND")) || IsPlatform(TEXT("IOS"))) return EFortCrucibleControlType::Touch;
                if (IsPlatform(TEXT("XBL")) || IsPlatform(TEXT("PSN")) || IsPlatform(TEXT("SWT"))) return EFortCrucibleControlType::Gamepad;
                return EFortCrucibleControlType::KBM;
            }
        }
    }
}
return CurrentControlType;
