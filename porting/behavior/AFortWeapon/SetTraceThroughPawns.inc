// Behaviour snippet: AFortWeapon::SetTraceThroughPawns (body only; spliced into the generated member function).
// Original address: 0x142b48900 (exec; the FFrame read and the store are in the raw decompile)
// Decompile source: porting/generated/native225-fresh-bulk/0x142b48900.c
// Proxy score: none (exec body verified against the raw decompile; no portability record)
// Inferred: the exec reads one parameter from the FFrame stack, then the byte at +0x89c is written with (argument != 0) (mov8). bool_bits.py maps (0x89c, 0x1) to bTraceThroughPawns.
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
bTraceThroughPawns = bNewTraceThroughPawns;
