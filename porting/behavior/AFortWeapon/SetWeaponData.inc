// Behaviour snippet: AFortWeapon::SetWeaponData (body only; spliced into the generated member function).
// Original address: 0x142b48b60 (exec; the FFrame read and the store are in the raw decompile)
// Decompile source: porting/generated/native225-fresh-bulk/0x142b48b60.c
// Proxy score: none (exec body verified against the raw decompile; no portability record)
// Inferred: the exec reads one parameter from the FFrame stack, then the 8-byte object pointer at +0x350 is stored from the argument. Reflection names +0x350 as WeaponData (Object, UFortWeaponItemDefinition*).
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
WeaponData = InWeaponData;
