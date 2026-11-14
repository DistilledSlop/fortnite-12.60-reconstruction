// CONFIDENCE: HIGH
// Original 1410fe3b0: load the actual backend at profile+0xd8, virtual +0x28.
// Required precondition: NativeBackend is installed and live. No null fallback.
return NativeBackend->FindItemsByTemplateKey(Key);
