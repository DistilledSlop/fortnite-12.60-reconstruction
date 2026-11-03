// Original 0x141df7530. NativeLoadGameData141df76b0 is a required
// provider; see recovery.md before integrating this body.
if (!GameDataCommon)
{
    GameDataCommon = Cast<UFortGameData>(NativeLoadGameData141df76b0(
        *this, UFortGameData::StaticClass(), GameDataNameCommon,
        UFortGameData::StaticClass()->GetFName()));
}
return GameDataCommon;
