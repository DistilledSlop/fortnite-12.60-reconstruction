// CONFIDENCE: high; native14191bde0 null-checks GetWorld then delegates to the engine replay predicate.
const UWorld* World = GetWorld();
return World && World->IsPlayingReplay();
