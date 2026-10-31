// Behaviour snippet: AFortPawn::IsDBNO (body only; spliced into the generated member function).
// Original address: 0x142a1cc80
// Decompile source: porting/generated/decomp/AFortPawn/IsDBNO.c
// Proxy score: 92.2 (portability.json)
// Inferred: Decompile reads byte +0x53a (1338) bit 0x2 (>> 1 & 1). bool_bits.py maps that exact (offset, mask) to bIsDBNO (or8 setter). Header: uint8 bIsDBNO : 1.
// Declared in generated header: no (generator must emit this declaration)
// CONFIDENCE: HIGH
return bIsDBNO;
