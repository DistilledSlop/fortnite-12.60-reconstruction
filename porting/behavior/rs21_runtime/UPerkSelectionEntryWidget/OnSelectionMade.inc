// Original body 0x144bfeac0 (thunk 0x145341730). Offset evidence: ItemA +0x270 and ItemB +0x278 (reflected Object); SelectionIndex int32 +0x268 (reflected, signed JL test); OnPerkSelected +0x258 (reflected) broadcast with the packed this pointer.
// CONFIDENCE: HIGH
if (ItemA != nullptr && ItemB != nullptr && SelectionIndex >= 0)
{
    OnPerkSelected.Broadcast(this);
}
