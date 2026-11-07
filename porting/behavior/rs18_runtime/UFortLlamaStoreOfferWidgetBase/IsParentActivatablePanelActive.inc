// Behaviour snippet: UFortLlamaStoreOfferWidgetBase::IsParentActivatablePanelActive (body only).
// Original body address: 0x1450b9760 (thunk 0x145285dc0). this_delta_total 0.
// Offset evidence: no reflected member covers this+0x20 on this class chain (lookup_offset.py), so the object offset is read raw (rule 3). The body walks the UObject outer link (+0x20) twice per loop step, starting at this. A null node returns false; a node that fails IsA(UUserWidget) returns false; a node that fails IsA(UCommonActivatablePanel) loops to the next step (0x1450b9772 via JNZ at 0x1450b97fa); otherwise it returns the byte at node+0x288 (raw, node class not reflected here). The third IsA test in the asm repeats the second on the same node and is folded.
// CONFIDENCE: MEDIUM
if (this == nullptr)
{
    return false;
}
UObject* Node = reinterpret_cast<UObject*>(this);
while (true)
{
    Node = *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(Node) + 0x20);
    Node = *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(Node) + 0x20);
    if (Node == nullptr)
    {
        return false;
    }
    if (!Node->IsA(UUserWidget::StaticClass()))
    {
        return false;
    }
    if (!Node->IsA(UCommonActivatablePanel::StaticClass()))
    {
        continue;
    }
    return *(reinterpret_cast<const uint8*>(Node) + 0x288) != 0;
}
