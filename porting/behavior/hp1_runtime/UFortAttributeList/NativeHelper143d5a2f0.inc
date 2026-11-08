// Helper 0x143d5a2f0, host UFortAttributeList (static).
// Evidence: MOV EAX,dword ptr [RCX+0x110]; RET.
// Reflection lists UUserWidget ColorAndOpacity at +0x110, which does not match an int32 count read; access follows the asm.
// Carousel callers (BeginAutoScrolling, NextPage) branch on the result compared with 1.
// CONFIDENCE: MEDIUM
return *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(Context) + 0x110);
