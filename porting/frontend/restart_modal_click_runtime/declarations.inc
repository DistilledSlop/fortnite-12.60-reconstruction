private:
    // Original byte +448; the constructor (0x145228430) stores 0. Its native setter (0x144f7b54f) is not reproduced here.
    uint8 bNativeRestartExitFlag448 = 0;
    // Original 0x141ed81a0 for its only caller here (restart requested): RestartApplication, then RequestExit(false) when the flag is set.
    static void NativeRestartApplicationOrExit141ed81a0(uint8 bExitIfNotRestarted);
