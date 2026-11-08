// Original body: 0x141b48a80 (thunk 0x142b842c0). Offset evidence: [RCX+0x708] is APhysicsObject::HealthSet (reflected Object, offset 0x708); the helper receives HealthSet + 0x58 (tail jump to 0x14070cc00); a null HealthSet returns 0.0f (XORPS).
// CONFIDENCE: HIGH
if (HealthSet == nullptr)
{
	return 0.0f;
}
return APhysicsObject::NativeHelper14070cc00(reinterpret_cast<uint8*>(HealthSet) + 0x58);
