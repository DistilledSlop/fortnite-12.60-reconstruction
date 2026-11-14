// CONFIDENCE: HIGH. Original 0x141bdd6e0; forwards the native sequence unchanged.
if (!LevelToStream.IsNone())
    return LevelToStream;
const FName Selected = NativeSelectBuildingLevel(Sequence);
if (Selected.IsNone())
{
    UE_LOG(LogFortWorld, Log, TEXT("%s.%s UNABLE to pick building to stream!"),
        *GetLevel()->GetFName().ToString(), *GetFName().ToString());
}
else
{
    UE_LOG(LogFortWorld, Verbose, TEXT("%s.%s picked %s to stream!"),
        *GetLevel()->GetFName().ToString(), *GetFName().ToString(), *Selected.ToString());
}
return Selected;
