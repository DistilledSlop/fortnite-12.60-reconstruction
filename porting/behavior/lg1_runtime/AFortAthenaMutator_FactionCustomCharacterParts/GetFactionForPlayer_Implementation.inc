// Original body address: 0x141ad7b70
// Evidence: MOVZX EAX,byte ptr [RBX+0x270]. Offset 0x270 is below this class's lowest own offset 0x278 and no reflected member of the class or its generated ancestors covers it; raw read.
// Verbosity-gated diagnostic log at call 0x142c54b70 elided (no state change).
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
return *reinterpret_cast<const uint8*>(Raw + 0x270);
