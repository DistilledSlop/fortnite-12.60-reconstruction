// Original body: 0x141cd9370 (thunk 0x14276b310). Offset evidence: byte +0x29a is compared as a whole byte against 0 (inside the declared bIsInteractable byte; read raw). TArray Data at +0x2b0 and Num at +0x2b8 (member fields NativeRecoveredUint64_0x2b0 and NativeRecoveredInt32_0x2b8), element stride 0xc: TWeakObjectPtr at +0, byte at +0x8, byte at +0xa.
// The first element whose weak pointer matches the controller (NativeWeakObjectPtrMatches1406a7e80) decides the result: true only when element +0x8 is nonzero and element +0xa is zero.
// CONFIDENCE: MEDIUM
static_assert(sizeof(AFortPlayerController) > 0, "InteractingController pointee is named for the UObject* conversion");
if (*(reinterpret_cast<const uint8*>(this) + 0x29a) == 0)
{
    return false;
}
if (InteractingController == nullptr)
{
    return false;
}

uint8* Element = reinterpret_cast<uint8*>(NativeRecoveredUint64_0x2b0);
uint8* ElementEnd = Element + static_cast<int64>(NativeRecoveredInt32_0x2b8) * 12;
for (; Element != ElementEnd; Element += 12)
{
    if (UConsolidatedQuestComponent::NativeWeakObjectPtrMatches1406a7e80(reinterpret_cast<TWeakObjectPtr<UObject>*>(Element), InteractingController))
    {
        if (Element == nullptr)
        {
            return false;
        }
        return *(Element + 0x8) != 0 && *(Element + 0xa) == 0;
    }
}
return false;
