// CONFIDENCE: HIGH
switch (Phase)
{
case 0: return TEXT("None");
case 1: return TEXT("Setup");
case 2: return TEXT("Warmup");
case 3: return TEXT("Aircraft");
case 4: return TEXT("SafeZones");
case 5: return TEXT("EndGame");
default: return TEXT("Unknown");
}
