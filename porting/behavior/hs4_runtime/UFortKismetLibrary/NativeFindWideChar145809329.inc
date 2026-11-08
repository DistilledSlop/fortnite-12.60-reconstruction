// helper 0x145809329 (UFortKismetLibrary): JMP qword ptr [0x1458fd598], the IAT thunk for VCRUNTIME140 wcschr.
// Arguments pass through unchanged (RCX = string, EDX = character); result is returned in RAX.
// CONFIDENCE: HIGH
return FCString::Strchr(Start, Find);
