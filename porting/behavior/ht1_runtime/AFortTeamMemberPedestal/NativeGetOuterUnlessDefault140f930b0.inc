// Helper 0x140f930b0 (AFortTeamMemberPedestal). Asm: MOV EAX,[RCX+8]; SHR EAX,4; TEST AL,1; JZ -> MOV RAX,[RCX+0x20]; RET ; else XOR EAX,EAX; RET.
// Evidence: +0x8 is the UObject ObjectFlags word, +0x20 is OuterPrivate (GetOuter()). Bit 0x10 is read as the default-object flag from the UObject layout, not from a reflected name.
// Caller evidence: GetCurrentTeamSize @0x144e329e0 passes TWeakObjectPtr::Get(this+0x3cc) in RCX and reads int32 +0x38 from the result.
// CONFIDENCE: MEDIUM

const int32 ObjectFlagWord = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(ObjectArg) + 0x8);
if (((ObjectFlagWord >> 4) & 0x1) != 0)
{
    return nullptr;
}
return ObjectArg->GetOuter();
