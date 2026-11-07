// Behaviour snippet: AFortWeaponRanged::IsCachedIsProjectileWeapon_Implementation (body only; spliced into the generated function).
// Original body address: 0x1426d24c0. Offset evidence: MOV EAX,dword [RCX+0xbd4]; SHR EAX,0xe; AND AL,1 = bit 14 of the dword at +0xbd4. bool_bits.json has no declared bit at byte +0xbd5 mask 0x40, so raw access is used.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint32 Packed = *reinterpret_cast<const uint32*>(Raw + 0xbd4);
return ((Packed >> 14) & 0x1u) != 0;
