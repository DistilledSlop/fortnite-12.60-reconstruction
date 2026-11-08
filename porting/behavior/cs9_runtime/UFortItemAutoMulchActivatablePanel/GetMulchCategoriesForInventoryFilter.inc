// Original body 0x144ebdfd0 (thunk 0x145267600). Offset evidence: weak object at this+0x448 (no reflected member in this class, raw access). Its object byte at +0x521 selects the categories: 0 gives {1,2}, 6 gives {5}, 7 gives {4}, 8 gives {3}, others give none; an invalid weak gives none.
// CONFIDENCE: HIGH
TArray<EFortAutoMulchCategory> Result;
const TWeakObjectPtr<UObject>* FilterWeak = reinterpret_cast<const TWeakObjectPtr<UObject>*>(reinterpret_cast<const uint8*>(this) + 0x448);
if (!FilterWeak->IsValid())
{
    return Result;
}
const uint8 FilterMode = *(reinterpret_cast<const uint8*>(FilterWeak->Get()) + 0x521);
if (FilterMode == 0)
{
    Result.Add(static_cast<EFortAutoMulchCategory>(1));
    Result.Add(static_cast<EFortAutoMulchCategory>(2));
}
else if (FilterMode == 6)
{
    Result.Add(static_cast<EFortAutoMulchCategory>(5));
}
else if (FilterMode == 7)
{
    Result.Add(static_cast<EFortAutoMulchCategory>(4));
}
else if (FilterMode == 8)
{
    Result.Add(static_cast<EFortAutoMulchCategory>(3));
}
return Result;
