// CONFIDENCE: HIGH
// Original Athena override +0x990. COMISS/JC selects the emote value for unordered inputs too.
const FFortPawnStats* Stats = NativeGetPawnStats141a07b70();
if (bIsDBNO)
    return Stats->SpeedDBNO;
const float Base = AFortPawn::NativeGetMinAnalogWalkSpeed14231a8f0();
if (bMovingEmote && !(EmoteWalkSpeed >= Base))
    return EmoteWalkSpeed;
return Base;
