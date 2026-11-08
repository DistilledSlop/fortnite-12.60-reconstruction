// Original body 0x14244a880 (thunk 0x142aba0f0): TArray<FString> (16-byte elements) at object +0xad0 with Num at +0xad8; not covered by a reflected member (only QuestMapMode at +0xac9). Moves the last element out, Num-- (the memmove branch is dead: its count is always 0), then calls the 16-byte slack shrink helper 0x140690e50. Returns an empty string when Num <= 0.
// CONFIDENCE: MEDIUM
static_assert(sizeof(AFortMission) > 0, "AFortMission declares the 16-byte array shrink helper");
FString PoppedString;
TArray<FString>& PendingUnlockQueue = *reinterpret_cast<TArray<FString>*>(reinterpret_cast<uint8*>(this) + 0xad0);
if (PendingUnlockQueue.Num() > 0)
{
    PoppedString = MoveTemp(PendingUnlockQueue[PendingUnlockQueue.Num() - 1]);
    PendingUnlockQueue.RemoveAt(PendingUnlockQueue.Num() - 1);
    AFortMission::NativeShrinkArray16ByteSlack140690e50(&PendingUnlockQueue);
}
return PoppedString;
