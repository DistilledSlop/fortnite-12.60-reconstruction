// CONFIDENCE: HIGH for logic, MEDIUM for the element field name. Original addresses: exec thunk 0x14283bb70,
// helper 0x141918950.
if (SpringIdx < 0 || SpringIdx >= Springs.Num())
{
    return 1.0f;   // 0x3f800000 returned for out-of-range indices
}
return Springs[SpringIdx].SpringLengthRatio;   // float at element offset 0xb8, element stride 0x160
