// Behaviour snippet: UBuildingStructuralSupportSystem::AreGridIndicesValid (body only).
// Original body address: 0x141c5fae0 (thunk 0x14275b6b0), receiver delta 0.
// Evidence: this+0x368, +0x36c, +0x370 are int32 grid limits (unreflected, own region): NativeRecoveredInt32_0x368/0x36c/0x370. GridIndices (FBuildingSupportCellIndex) is read as three int32 at +0/+4/+8. Returns false when any index exceeds its limit, else true.
// CONFIDENCE: HIGH
const int32* Indices = reinterpret_cast<const int32*>(&GridIndices);
if (Indices[0] > NativeRecoveredInt32_0x368 || Indices[1] > NativeRecoveredInt32_0x36c || Indices[2] > NativeRecoveredInt32_0x370)
{
    return false;
}
return true;
