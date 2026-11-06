// CONFIDENCE: high
// 0x1415f7dd0: resolve and cast the avatar only when actor info exists.
return CurrentActorInfo ? Cast<AFortPawn>(CurrentActorInfo->AvatarActor.Get()) : nullptr;
