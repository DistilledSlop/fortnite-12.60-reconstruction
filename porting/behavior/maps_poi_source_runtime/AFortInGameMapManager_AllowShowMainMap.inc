// CONFIDENCE: HIGH
{
    if (NativeMainMapVisible_0x231 != static_cast<uint8>(bAllowShow))
    {
        NativeMainMapVisible_0x231 = bAllowShow;
        OnAllowShowMainMapChanged.Broadcast(bAllowShow);
    }
}
