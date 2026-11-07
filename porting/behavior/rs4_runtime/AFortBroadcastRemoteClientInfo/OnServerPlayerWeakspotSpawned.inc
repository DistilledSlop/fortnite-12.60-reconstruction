// Original body 0x1424b5bc0 (thunk 0x14284f7f0). Offset evidence: this+0xf0 = AActor::Role (reflected Byte, ROLE_Authority == 3); this+0x268 = bActive (reflected bool, byte compared with 0); this+0x2a8..0x2df = RemoteWeakspotData (FBuildingWeakSpotData, 56 bytes, copied from WeakspotData by 3 MOVUPS and 1 MOVSD).
// CONFIDENCE: HIGH
if (static_cast<uint8>(GetLocalRole()) != 3 || !bActive)
{
	return;
}
RemoteWeakspotData = WeakspotData;
