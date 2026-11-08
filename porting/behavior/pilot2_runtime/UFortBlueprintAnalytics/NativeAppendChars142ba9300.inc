// Helper 0x142ba9300: FString::AppendChars(const WIDECHAR* Str, int32 Count). Asm: `TEST R8D,R8D; JZ` (Count == 0 returns),
// NewNum = OldNum + (OldNum == 0 ? 1 : 0) + Count stored at [RCX+8], regrow via 0x1406800e0 (TArray ResizeGrow) when above Max,
// memcpy(Data + OldNum - (OldNum != 0), Str, Count * 2) (0x142bc7780), then a 0 TCHAR terminator after the copied chars.
TargetString->AppendChars(Chars, CharCount);
// CONFIDENCE: HIGH
