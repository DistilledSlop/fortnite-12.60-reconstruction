// Original address: 0x14191bdb0 (AFortAthenaVehicle::IsMagnetizedForSkyTubes)
// Offset evidence: [RCX+0xaf8] is SkyTubePhysicsComponent (reflected Object); byte +0x108 on the pointee read raw (far path taken from decompile FUN_14191bdb0; far asm not in insns).
// CONFIDENCE: MEDIUM
if (SkyTubePhysicsComponent == nullptr)
{
    return true;
}
return *(reinterpret_cast<const uint8*>(SkyTubePhysicsComponent) + 0x108) != 0;
