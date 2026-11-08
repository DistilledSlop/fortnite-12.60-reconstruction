// Native 0x142ee43c0: decrements the int32 counter at +0x0C of the struct and returns the new value.
// Asm evidence: MOV EAX,[RCX+0xc]; DEC EAX; MOV [RCX+0xc],EAX; RET (RAX holds the new value at return).
// Caller asm was not located, so the return value is the literal RAX result of the DEC.
// CONFIDENCE: MEDIUM
int32* CounterSlot = reinterpret_cast<int32*>(Self + 0xC);
*CounterSlot = *CounterSlot - 1;
return *CounterSlot;
