// Helper 0x142119f50 (host AFortWeapon). Caller: AFortWeapon::GetDamageStartLocation (body 0x14269bad0), which passes RCX=this, RDX=OutFirst, R8 = its own third argument (OutSecond).
// Evidence: two XOR-keyed FVector records are decoded from the weapon.
//   Record A: index byte [this+0xa3a], key byte [this+0xa3b]; key = keybyte|0x4c000001 for X and Y, keybyte|0x0a001000 for Z.
//     Record at this + index*12. X = dword[+0xa0e]^keyA, Y = dword[+0xa0a]^keyA, Z = dword[+0xa12]^(keybyte|0x0a001000).
//   Record B: index byte [this+0xa08], key byte [this+0xa09]; same key rules.
//     Record at this + index*12. X = dword[+0x9dc]^keyB, Y = dword[+0x9d8]^keyB, Z = dword[+0x9e0]^(keybyte|0x0a001000).
// The stack cookie check (CALL 0x145803c70) and XOR RAX,RSP pair are omitted.
// Offsets are not covered by any reflected AFortWeapon member, so raw access is used.
// Values are written as raw float bit patterns, exactly as the asm stores them.
const uint8* WeaponBytes = reinterpret_cast<const uint8*>(WeaponObj);

const uint32 RecordIndexA = WeaponBytes[0xa3a];
const uint32 KeyByteA = WeaponBytes[0xa3b];
const uint32 KeyWordA = KeyByteA | 0x4c000001u;
const uint8* RecordA = WeaponBytes + RecordIndexA * 12;
const uint32 BitsAX = *reinterpret_cast<const uint32*>(RecordA + 0xa0e) ^ KeyWordA;
const uint32 BitsAY = *reinterpret_cast<const uint32*>(RecordA + 0xa0a) ^ KeyWordA;
const uint32 BitsAZ = *reinterpret_cast<const uint32*>(RecordA + 0xa12) ^ (KeyByteA | 0x0a001000u);

const uint32 RecordIndexB = WeaponBytes[0xa08];
const uint32 KeyByteB = WeaponBytes[0xa09];
const uint32 KeyWordB = KeyByteB | 0x4c000001u;
const uint8* RecordB = WeaponBytes + RecordIndexB * 12;
const uint32 BitsBX = *reinterpret_cast<const uint32*>(RecordB + 0x9dc) ^ KeyWordB;
const uint32 BitsBY = *reinterpret_cast<const uint32*>(RecordB + 0x9d8) ^ KeyWordB;
const uint32 BitsBZ = *reinterpret_cast<const uint32*>(RecordB + 0x9e0) ^ (KeyByteB | 0x0a001000u);

*reinterpret_cast<uint32*>(&OutFirst.X) = BitsAX;
*reinterpret_cast<uint32*>(&OutFirst.Y) = BitsAY;
*reinterpret_cast<uint32*>(&OutFirst.Z) = BitsAZ;
*reinterpret_cast<uint32*>(&OutSecond.X) = BitsBX;
*reinterpret_cast<uint32*>(&OutSecond.Y) = BitsBY;
*reinterpret_cast<uint32*>(&OutSecond.Z) = BitsBZ;
// CONFIDENCE: HIGH
