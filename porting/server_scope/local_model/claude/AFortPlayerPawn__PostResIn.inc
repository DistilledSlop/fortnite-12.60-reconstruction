// CONFIDENCE: HIGH for control flow; LOW for the event FName (data constant 0x14817bcd8, not resolved). Original addresses: exec 0x142a7e380; helper 0x14277c110 (read in full: FindFunctionChecked then ProcessEvent with no parameters).
// Each non-null character part modifier (6 slots) receives the parameterless PostResIn event.
for (int32 PartIndex = 0; PartIndex < 6; ++PartIndex)
{
    UObject* PartModifier = CharacterPartModifiers[PartIndex];
    if (PartModifier)
    {
        UFunction* PostResInEvent = PartModifier->FindFunctionChecked(PostResInEventName);   // FName @0x14817bcd8 (LOW)
        PartModifier->ProcessEvent(PostResInEvent, nullptr);
    }
}
