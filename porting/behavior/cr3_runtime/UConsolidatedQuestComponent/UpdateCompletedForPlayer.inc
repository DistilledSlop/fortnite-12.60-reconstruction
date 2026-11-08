// Original body: 0x141cf5ce0 (thunk 0x14276c290). Offset evidence: TArray Data at +0x2b0 and Num at +0x2b8 (member fields NativeRecoveredUint64_0x2b0 and NativeRecoveredInt32_0x2b8), element stride 0xc: TWeakObjectPtr at +0, byte at +0xa. The first element whose weak pointer matches InPlayerController (NativeWeakObjectPtrMatches1406a7e80) gets bCompleted stored to byte +0xa, then the loop exits.
// CONFIDENCE: MEDIUM
static_assert(sizeof(AFortPlayerController) > 0, "InPlayerController pointee is named for the UObject* conversion");
if (InPlayerController == nullptr)
{
    return;
}

uint8* Element = reinterpret_cast<uint8*>(NativeRecoveredUint64_0x2b0);
uint8* ElementEnd = Element + static_cast<int64>(NativeRecoveredInt32_0x2b8) * 12;
for (; Element != ElementEnd; Element += 12)
{
    if (UConsolidatedQuestComponent::NativeWeakObjectPtrMatches1406a7e80(reinterpret_cast<TWeakObjectPtr<UObject>*>(Element), InPlayerController))
    {
        if (Element == nullptr)
        {
            return;
        }
        *(Element + 0xa) = static_cast<uint8>(bCompleted);
        return;
    }
}
