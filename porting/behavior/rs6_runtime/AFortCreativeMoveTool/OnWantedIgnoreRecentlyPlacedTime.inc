// Behaviour snippet: AFortCreativeMoveTool::OnWantedIgnoreRecentlyPlacedTime (body only; spliced into the generated member function).
// Original address: 0x14267d6f0
// Offset evidence: null CreativeOption returns without store (JZ to RET). Otherwise the int32 result of UFortCreativeOption::GetValueAtIndex(Index) is stored at +0x1188, which is the Value field (+8) of WantedIgnoreRecentlyPlacedTime (FCreativeOptionVariableBase at +0x1180).
// CONFIDENCE: HIGH
if (CreativeOption == nullptr)
{
	return;
}
WantedIgnoreRecentlyPlacedTime.Value = CreativeOption->GetValueAtIndex(static_cast<int32>(Index));
