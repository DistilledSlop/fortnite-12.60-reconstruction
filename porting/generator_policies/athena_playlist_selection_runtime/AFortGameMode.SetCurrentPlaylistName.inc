    // CONFIDENCE: HIGH
    // 0x141e7ea80: this slot only diagnoses client-side use; it never assigns.
    if (GetWorld()->GetNetMode() == NM_Client)
    {
        UE_LOG(LogFortPlaylistSelection, Warning, TEXT("AFortGameMode::SetCurrentPlaylistName can only be used in Athena currently"));
    }
