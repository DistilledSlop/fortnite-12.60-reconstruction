// Behaviour snippet: AFortCreativeMoveTool::OnWantedCameraSpacePositioning (body only; spliced into the generated member function).
// Original address: 0x14267d650
// Offset evidence: null CreativeOption returns without store (JZ to RET). Otherwise the int32 result of UFortCreativeOption::GetValueAtIndex(Index) is stored at +0x11a4, which is the Value field (+8) of WantedCameraSpacePositioning (FCreativeOptionVariableBase at +0x119c).
// CONFIDENCE: HIGH
if (CreativeOption == nullptr)
{
	return;
}
WantedCameraSpacePositioning.Value = CreativeOption->GetValueAtIndex(static_cast<int32>(Index));
