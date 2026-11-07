// Original body 0x141922220 (thunk 0x142834c40). Offset evidence: no receiver or argument field is read. The only calls are AFortGameModePickup::StaticClass() probes (0x1428f0dd0) whose results never reach the return value; every path ends with XOR EAX,EAX (null).
// CONFIDENCE: HIGH
return nullptr;
