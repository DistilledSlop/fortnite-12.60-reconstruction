// CONFIDENCE: HIGH. Authority-only state change: flush net dormancy, store the state, run the OnRep, then refresh net dormancy through the post-change hook.
// Original address: 0x142744630 (exec). Native body: 0x141beb220.
if (GetLocalRole() == ROLE_Authority && NewState != SpawnMachineState)
{
    FlushNetDormancy();                 // native 0x143f72040 (AActor::FlushNetDormancy)
    SpawnMachineState = NewState;       // byte at +0x7b2, replicated with OnRep_SpawnMachineState
    OnRep_SpawnMachineState();          // native 0x142742ff0 (ProcessEvent of the rep notify)
    UpdateNetDormancyState();           // virtual slot 0x5e8 on this, target 0x143f72550
}
