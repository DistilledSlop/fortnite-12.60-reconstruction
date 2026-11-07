// Behaviour snippet: UCommonInputSubsystem::GetCurrentInputType (body only; spliced into the generated member function).
// Original address: 0x14081b540
// Evidence: asm MOVZX EAX,byte [RCX+0x79]; RET. Member CurrentInputType (ECommonInputType, enum byte) at +0x79 (reflection offset 121, UCommonInputSubsystem.h).
// CONFIDENCE: HIGH
return CurrentInputType;
