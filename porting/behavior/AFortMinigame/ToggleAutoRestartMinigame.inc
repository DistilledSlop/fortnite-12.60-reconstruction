// Original address: 0x1429c6810; native body 0x1420a9070 (exec thunk tail-jumps there after Stack.Code bump).
// Decompile source: porting/generated/decomp/AFortMinigame/ToggleAutoRestartMinigame__1429c6810.c
// Inferred: only on the authority role (AActor Role byte at +0xf0 == ROLE_Authority, 3), the unnamed flag byte at +0xab1 is set to (flag == 0), i.e. toggled. The flag is declared as member_field storage (NativeRecoveredBool_0xab1).
// CONFIDENCE: HIGH
if (GetLocalRole() == ROLE_Authority)
{
    NativeRecoveredBool_0xab1 = !NativeRecoveredBool_0xab1;
}
