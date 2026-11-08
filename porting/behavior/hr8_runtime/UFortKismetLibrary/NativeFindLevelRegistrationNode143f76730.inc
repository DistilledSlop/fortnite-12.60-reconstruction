// Helper 0x143f76730: PUSH RBX; MOV RBX,RCX; CALL 0x14477bcd0 (ULevel::StaticClass); MOV RDX,RAX; MOV RCX,RBX; JMP 0x142eca940.
// Tail call into the ported helper 0x142eca940 with the original context and ULevel::StaticClass() as second argument.
// CONFIDENCE: HIGH - the body is exactly the tail call; the return value is the callee's.
return UFortClientSettingsRecord::NativeFindRegistrationNode142eca940(Context, reinterpret_cast<uint8*>(ULevel::StaticClass()));
