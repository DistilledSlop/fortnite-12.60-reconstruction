// Helper 0x1412b1090 (host UFortFrontEndContext). Asm: MOVZX EAX,byte ptr [RCX + 0xba]; RET.
// Returns the raw uint8 at receiver offset 0xba. Callers pass the receiver in RCX and use AL as a byte value
// (0x1425720a0 compares AL with 0x11, 0x144ce6fd0 compares AL with R13B, others copy AL).
// lookup_offset.py reports reflected OnTheaterPinClicked at +0xb8 as the nearest member, whose width is not
// reflected, so the byte is read raw with the asm width (uint8).
// CONFIDENCE: HIGH
return *(reinterpret_cast<const uint8*>(ContextObject) + 0xBA);
