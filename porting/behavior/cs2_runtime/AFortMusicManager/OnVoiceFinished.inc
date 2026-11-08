// Behaviour snippet: AFortMusicManager::OnVoiceFinished (body 0x1424b65c0, thunk 0x1429fc160).
// Offset evidence: VoicesToCleanUp TArray<UFortMusicVoice*> reflected at object +0x238; Num read at +0x240 (raw write for the decrement). Linear search for Voice; on hit, memmove the tail down one slot, Num -= 1, then shrink slack via 0x14072c6c0 (NativeArrayShrinkSlack14072c6c0).
// Asm also contains an index == -1 check after a successful search, which is unreachable (the index is always >= 0 there) and is omitted.
// CONFIDENCE: HIGH
static_assert(sizeof(AAIHotSpot) > 0, "AAIHotSpot must be complete for the static shrink helper");
const int32 ArrayNum = VoicesToCleanUp.Num();
UFortMusicVoice** Elements = VoicesToCleanUp.GetData();
int32 FoundIdx = INDEX_NONE;
for (int32 i = 0; i < ArrayNum; ++i)
{
    if (Elements[i] == Voice)
    {
        FoundIdx = i;
        break;
    }
}
if (FoundIdx == INDEX_NONE)
{
    return;
}
const int32 RemainingCount = ArrayNum - FoundIdx - 1;
if (RemainingCount != 0)
{
    FMemory::Memmove(&Elements[FoundIdx], &Elements[FoundIdx + 1], static_cast<SIZE_T>(RemainingCount) * sizeof(UFortMusicVoice*));
}
*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(this) + 0x240) = ArrayNum - 1;
AAIHotSpot::NativeArrayShrinkSlack14072c6c0(static_cast<void*>(&VoicesToCleanUp));
