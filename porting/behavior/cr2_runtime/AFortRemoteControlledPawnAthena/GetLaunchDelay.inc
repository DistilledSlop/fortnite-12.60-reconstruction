// Original body: 0x14243fe70 (thunk 0x142ac23a0). Offset evidence: [RCX+0x30d8] is AFortRemoteControlledPawnAthena::RemoteControlPawnSet (reflected Object, offset 0x30d8); the helper receives that pointer + 0x1e8 (tail jump to 0x14070cc00); a null pointer returns 0.0f (XORPS).
// CONFIDENCE: HIGH
if (RemoteControlPawnSet == nullptr)
{
	return 0.0f;
}
return APhysicsObject::NativeHelper14070cc00(reinterpret_cast<uint8*>(RemoteControlPawnSet) + 0x1e8);
