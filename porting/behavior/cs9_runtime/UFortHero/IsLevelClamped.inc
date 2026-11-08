// Original body 0x142521520 (thunk 0x14293c430). Offset evidence: this+0x228 int32 lies after CharacterParts (0x218, TArray, 16 bytes) in the own region, recorded as NativeRecoveredInt32_0x228. Callee 0x1407c1f50 is the hero level helper taking the hero.
// CONFIDENCE: HIGH
const int32 ClampLevel = NativeRecoveredInt32_0x228;
if (ClampLevel < 0)
{
    return false;
}
return ClampLevel < UFortHero::NativeGetHeroLevel1407c1f50(const_cast<UFortHero*>(this));
