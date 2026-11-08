// Original body: 0x141b47490 (thunk 0x142b84290). Offset evidence: [RCX+0x708] is APhysicsObject::HealthSet (reflected Object, offset 0x708); the helper receives HealthSet + 0x30 (tail jump to 0x14070cc00); a null HealthSet returns 0.0f (XORPS).
// CONFIDENCE: HIGH
if (HealthSet == nullptr)
{
	return 0.0f;
}
return APhysicsObject::NativeHelper14070cc00(reinterpret_cast<uint8*>(HealthSet) + 0x30);
