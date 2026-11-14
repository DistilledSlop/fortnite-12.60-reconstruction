// Original address: 0x14234dae0
// CONFIDENCE: high
const bool bChanged = (bNewValue != static_cast<bool>(bIsSlopeSliding));
if (bChanged)
{
    OnRep_IsSlopeSliding();
}
bIsSlopeSliding = bNewValue ? 1 : 0;
