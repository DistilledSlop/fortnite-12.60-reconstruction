// Behaviour snippet: AFortWeaponRanged::SetShellReloadCounter (body only; spliced into the generated member function).
// Original address: 0x142b50130 (exec; the FFrame read and the store are in the raw decompile)
// Decompile source: porting/generated/native225-fresh-bulk/0x142b50130.c
// Proxy score: none (exec body verified against the raw decompile; no portability record)
// Inferred: the exec reads one parameter from the FFrame stack, then the 32-bit int at +0xce4 is stored from the argument. Reflection names +0xce4 as ShellReloadCounter (Int).
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
ShellReloadCounter = InputShellReloadCounter;
