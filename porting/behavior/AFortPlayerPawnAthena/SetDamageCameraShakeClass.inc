// Behaviour snippet: AFortPlayerPawnAthena::SetDamageCameraShakeClass (body only; spliced into the generated member function).
// Original address: 0x142a83d20
// Decompile source: porting/generated/decomp/AFortPlayerPawnAthena/SetDamageCameraShakeClass.c
// Proxy score: 86.2 (portability.json)
// Inferred: Decompile stores the 8-byte argument unchanged to +0x3070 (DamageCameraShakeClass, TSubclassOf<UCameraShake> in the header). No class check is made in the decompile, so the cast is a raw pointer store.
// Declared in generated header: no (generator must emit this declaration)
// CONFIDENCE: MEDIUM
DamageCameraShakeClass = static_cast<UClass*>(NewValue);
