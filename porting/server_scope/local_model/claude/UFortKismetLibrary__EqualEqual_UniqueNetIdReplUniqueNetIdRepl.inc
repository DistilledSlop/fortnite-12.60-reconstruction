// CONFIDENCE: HIGH
// Native 0x142969ef0 (thunk 0x140d7c5a0): both ids must be valid or both invalid; two invalid ids compare equal;
// two valid ids compare through FUniqueNetIdRepl equality.
if (A.IsValid() != B.IsValid())
{
    return false;
}
if (!A.IsValid())
{
    return true;
}
return A == B;
