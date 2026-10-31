// Behaviour snippet: AFortPlayerControllerAthena::ClientBroadcastOnRestoreCreativePlotFinished_Implementation (body only; spliced into the generated member function).
// Original address: 0x1419c54a0
// Decompile source: porting/generated/decomp_improved/AFortPlayerControllerAthena/ClientBroadcastOnRestoreCreativePlotFinished_Implementation.c
// Proxy score: n/a (RPC body, not in the exec screen)
// Inferred: the body builds the parameter block { bool bSuccess @0; FText Reason @8 } (FUN_142c0c240 = FText::FText, FUN_14079d060 = FText::operator=), then broadcasts it through the dispatcher 0x1406a9dc0 (TMulticastScriptDelegate::ProcessMulticastDelegate) on the member at +0x2df0, which is OnRestoreCreativePlotFinished (reflection offset 11760). The release code after the call is the destructor of the local FText. Parameters are in declaration order (bSuccess, Reason), as the generated header declares them.
// Declared in generated header: yes
// CONFIDENCE: HIGH
OnRestoreCreativePlotFinished.Broadcast(bSuccess, Reason);
