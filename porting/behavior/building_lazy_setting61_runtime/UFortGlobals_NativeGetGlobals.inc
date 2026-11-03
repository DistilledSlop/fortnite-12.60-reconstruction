// CONFIDENCE: HIGH
// Native 0x141ed2350: checked live GameSingleton, fatal diagnostic and NewObject fallback.
UFortGlobals* Globals = Cast<UFortGlobals>(GEngine->GameSingleton);
if (Globals)
{
    return Globals;
}
UE_LOG(LogFort, Fatal, TEXT("Invalid Engine used, must be FortEngine or FortUnrealEdEngine"));
return NewObject<UFortGlobals>();
