// Helper 0x142681e90 on AFortCreativeMoveTool. Caller: OnWantsPrecisionGridSnapping (0x14267d760) with RCX=MoveTool and DL=requested index.
// Evidence: MaxIndex = uint8([MoveTool+0x1140]) - 1 (byte wrap); GridSnapIndex [+0x1129] = min(requested, MaxIndex) (CMOVC on DL < R8B).
// Then AFortCreativeMoveTool::NativeGetInstigatorController143f76650 must be non-null and an AFortPlayerController (IsA via StaticClass 0x142a3d9f0);
// controller byte [+0x14ab] = GridSnapIndex.
// CONFIDENCE: HIGH
const uint8 RawPrecision = *reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(MoveTool) + 0x1140);
const uint8 MaxSnapIndex = static_cast<uint8>(RawPrecision - 1);
MoveTool->GridSnapIndex = RequestedIndex < MaxSnapIndex ? RequestedIndex : MaxSnapIndex;
AController* InstigatorController = AFortCreativeMoveTool::NativeGetInstigatorController143f76650(MoveTool);
if (InstigatorController == nullptr)
{
	return;
}
if (!InstigatorController->IsA(AFortPlayerController::StaticClass()))
{
	return;
}
*reinterpret_cast<uint8*>(reinterpret_cast<uint8*>(InstigatorController) + 0x14ab) = MoveTool->GridSnapIndex;
