// Behaviour snippet: UFortMiniMapComponent::OverrideLocalMiniMapIndicatorIsVisible (body only; spliced into the generated member function).
// Original address: 0x1425d2b80
// Evidence: TEST dword [RCX+0xe0],0x100 = bit 8 at 0xe0 = MiniMapData (FFortMiniMapData at 0xd0) +0x10 byte bit0 of byte 1 = 9th uint8 bitfield bAllowLocalOverrides. MOV byte [RCX+0x194],1 and [RCX+0x195],DL are unreflected own-region gaps (NativeRecoveredBool_0x194/0x195).
// CONFIDENCE: MEDIUM
if (!MiniMapData.bAllowLocalOverrides)
{
	return;
}
NativeRecoveredBool_0x194 = true;
NativeRecoveredBool_0x195 = bVisible;
