const AFortPawn* Pawn = ActorInfo ? Cast<AFortPawn>(ActorInfo->AvatarActor.Get()) : nullptr;
return (!Pawn || !Pawn->bIsDBNO) &&
    Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
