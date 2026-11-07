// Original address: 0x14190de30 (AFortAthenaVehicle::BlocksBuilding, receiver delta 944)
// Offset evidence: object offset 0x738 is an unreflected qword (NativeRecoveredUInt64_0x738) loaded from [RCX+0x388]; byte +0xac on the pointee read raw.
// Null path returns 0 per asm (EAX zeroed with RAX) and decompile FUN_14190de30.
// CONFIDENCE: MEDIUM
const uint8* Pointee = reinterpret_cast<const uint8*>(NativeRecoveredUInt64_0x738);
if (Pointee == nullptr)
{
    return false;
}
return *(Pointee + 0xac) != 0;
