// Original address: 0x14514f610; director_player_root_runtime/manager-world.definitions.inc. CONFIDENCE: MEDIUM.
UGameInstance* GameInstance = Cast<UGameInstance>(GetOuter()->GetOuter());
    return GameInstance->UGameInstance::GetWorld();
