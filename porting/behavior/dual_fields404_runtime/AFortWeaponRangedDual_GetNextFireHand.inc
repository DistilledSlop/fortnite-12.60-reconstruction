// CONFIDENCE: high; native wrapper compares the last-hand byte with zero and returns the resulting enum byte.
{
    return static_cast<EDualWeaponHand>(NativeRecoveredByte_0xf40 == 0);
}
