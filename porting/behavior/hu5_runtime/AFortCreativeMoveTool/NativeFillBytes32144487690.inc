// Helper 0x144487690 (host AFortCreativeMoveTool, static). Fills a 32-byte buffer with FillByte and reports whether any byte changed.
// Evidence: RCX = Bytes (32-byte buffer), DL = FillByte. Caller 0x142099a10 @0x142099ad4 sets LEA RCX,[RSP+0x78] and MOV EDX,0x1 before the call.
// The loop runs once (EBX 0 -> 0x20). Each element is compared with DL and stored only when different; R8B accumulates the changed flag.
// Return: MOVZX EAX,R8B.
// CONFIDENCE: HIGH
bool bChanged = false;
for (int32 ByteIndex = 0; ByteIndex < 32; ++ByteIndex)
{
	if (Bytes[ByteIndex] != FillByte)
	{
		Bytes[ByteIndex] = FillByte;
		bChanged = true;
	}
}
return bChanged;
