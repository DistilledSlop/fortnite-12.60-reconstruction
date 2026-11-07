// Original body address: 0x144dbc9e0
// Offset evidence: Pointer MainSizeBox (reflected, +0x488). Pointee float at +0x130 (pointee class unresolved, raw). Null path returns the float constant -1.0f read from runtime dump .rdata at 0x1459a979c.
// CONFIDENCE: MEDIUM
const uint8* SizeBox = reinterpret_cast<const uint8*>(MainSizeBox);
if (SizeBox == nullptr)
{
    return -1.0f;
}
return *reinterpret_cast<const float*>(SizeBox + 0x130);
