// Original body 0x141f3f280 (thunk 0x142aa2640): LinkDataErrorStatus is the reflected FString at +0x200 (Num at +0x208). Broadcasts OnLinkDataError (+0xc0) with a copy only when Num > 1; the asm guard JLE 0x141f3f318 is the epilogue.
// CONFIDENCE: HIGH
if (LinkDataErrorStatus.Num() > 1)
{
    FString LinkDataErrorCopy = LinkDataErrorStatus;
    OnLinkDataError.Broadcast(LinkDataErrorCopy);
}
