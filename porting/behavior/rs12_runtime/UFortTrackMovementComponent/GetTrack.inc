// Original body 0x1420fa850 (thunk 0x142b196e0). Offset evidence: reflected TrackMovement Struct at +0x140, but the asm loads an 8-byte pointer there, so raw access is used; that pointer's +0xa0 qword is the candidate object; IsA(AFortTrack::StaticClass()) via call 0x142b158c0 (ClassPrivate at +0x10).
// Returns the object when it is an AFortTrack, otherwise null.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* Holder = *reinterpret_cast<const uint8* const*>(Raw + 0x140);
if (Holder == nullptr)
{
    return nullptr;
}
UObject* Track = *reinterpret_cast<UObject* const*>(Holder + 0xa0);
if (Track == nullptr)
{
    return nullptr;
}
return Cast<AFortTrack>(Track);
