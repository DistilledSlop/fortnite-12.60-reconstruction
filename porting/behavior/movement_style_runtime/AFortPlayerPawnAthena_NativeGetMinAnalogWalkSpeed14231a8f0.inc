// CONFIDENCE: HIGH
// Original Athena override +0x990. COMISS/JC selects the emote value for unordered inputs too.
if (bIsDBNO)
    return NativeGetMovementSpeed14231ab10(0);
const float Base = AFortPawn::NativeGetMinAnalogWalkSpeed14231a8f0();
if (bMovingEmote && !(EmoteWalkSpeed >= Base))
    return EmoteWalkSpeed;
return Base;
