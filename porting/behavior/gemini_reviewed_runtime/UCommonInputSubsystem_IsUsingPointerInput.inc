// Behaviour snippet: UCommonInputSubsystem::IsUsingPointerInput (body only; spliced into the generated member function).
// Original address: 0x14081c1d0 (via exec wrapper 0x14081f030)
// Source: local-model fn runner; static checks only, not compiled or runtime verified.
// CONFIDENCE: HIGH
return (static_cast<uint8>(LastInputType) & 0xfd) == 0;
