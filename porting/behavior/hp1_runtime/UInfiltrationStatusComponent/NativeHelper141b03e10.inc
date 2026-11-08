// Helper 0x141b03e10, host UInfiltrationStatusComponent (static).
// Evidence: LEA RAX,[RCX+0x788]; RET. Returns the address of the +0x788 sub-object; no load is performed.
// Callers pass TWeakObjectPtr::Get(this+0x100) results and use the returned pointer as the state block.
// CONFIDENCE: HIGH
return reinterpret_cast<uint8*>(Context) + 0x788;
