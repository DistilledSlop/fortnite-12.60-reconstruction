// Behaviour snippet: AFortRemoteControlledPawnAthena::GetOutOfHealthExplodeDelay (body only; spliced into the generated member function).
// Original body address: 0x142440fe0 (thunk 0x142ac23d0)
// Offset evidence: this+0x30d8 = RemoteControlPawnSet (Object, reflected). The helper context is that pointer + 0x238 (ADD RCX,0x238 before the tail jump).
// Null path returns the .rdata float at 0x1459a86fc (bytes 00004040 = 3.0f).
// CONFIDENCE: HIGH
static_assert(sizeof(APhysicsObject) > 0, "APhysicsObject must be complete");
if (RemoteControlPawnSet != nullptr)
{
    return APhysicsObject::NativeHelper14070cc00(reinterpret_cast<uint8*>(RemoteControlPawnSet) + 0x238);
}
return 3.0f;
