// Behaviour snippet: UFortPickerContext::GetPickerWheelAdjacency (body only; spliced into the generated member function).
// Original address: 0x144d06f50 (thunk 0x1452a4a60, RDX = bCanPageLeft, R8 = bCanPageRight per the header order)
// Offset evidence: AvailableWheels is a reflected TArray at +0x138 (Num at +0x140, UFortPickerContext.h). CurrentWheelIndex is a reflected int32 at +0x148 (UFortPickerContext.h).
// Asm: bCanPageLeft = (CurrentWheelIndex - 1) in [0, Num); bCanPageRight = (CurrentWheelIndex + 1) in [0, Num); signed SUB/ADD with JS and CMP/JGE, which matches IsValidIndex.
// CONFIDENCE: HIGH
bCanPageLeft = AvailableWheels.IsValidIndex(CurrentWheelIndex - 1);
bCanPageRight = AvailableWheels.IsValidIndex(CurrentWheelIndex + 1);
