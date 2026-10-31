// Behaviour snippet: AFortPlayerControllerAthena::IsClientNotifiedOfLoss (body only; spliced into the generated member function).
// Original address: 0x142a58e20
// Decompile source: porting/generated/decomp/AFortPlayerControllerAthena/IsClientNotifiedOfLoss.c
// Proxy score: 92.5 (portability.json)
// Inferred: Decompile returns the whole byte +0x2c35. bool_bits.py maps that byte to bClientNotifiedOfLoss alone (mov8 setter, which writes the whole byte), so the byte equals the flag.
// Declared in generated header: no (generator must emit this declaration)
// CONFIDENCE: MEDIUM
return bClientNotifiedOfLoss;
