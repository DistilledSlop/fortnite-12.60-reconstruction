{
    // CONFIDENCE: HIGH. The native wrapper tests bit 0 at 0x2bf0 and the byte at 0x2ae5.
    return (NativeRecoveredBits_0x2bf0 & 1) != 0 && NativeRecoveredBool_0x2ae5;
}
