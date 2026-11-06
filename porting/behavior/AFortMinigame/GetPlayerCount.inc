// CONFIDENCE: high
// Native 0x1420930f0; wrapper 0x1429c0a40.
{
    return PlayerArray.Players.Num() + (bIncludeSpectators ? SpectatingPlayerArray.Players.Num() : 0);
}
