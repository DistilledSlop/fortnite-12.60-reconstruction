// Behaviour snippet: AFortPlayerControllerAthena::ClientBroadcastOnMakeNewCreativePlotFinished_Implementation (body only; spliced into the generated member function).
// Original address: 0x1419c5490
// Decompile source: porting/generated/decomp_improved/AFortPlayerControllerAthena/ClientBroadcastOnMakeNewCreativePlotFinished_Implementation.c
// Proxy score: n/a (RPC body, not in the exec screen)
// Inferred: the body builds the parameter block { bool bSuccess @0; FText Reason @8 } (FUN_142c0c240 = FText::FText, FUN_14079d060 = FText::operator=), then broadcasts it through the dispatcher 0x1406a9dc0 (TMulticastScriptDelegate::ProcessMulticastDelegate) on the member at +0x2dd0, which is OnMakeNewCreativePlotFinished (reflection offset 11728). The release code after the call is the destructor of the local FText. Parameters are in declaration order (bSuccess, Reason), as the generated header declares them.
// Declared in generated header: yes
// CONFIDENCE: HIGH
OnMakeNewCreativePlotFinished.Broadcast(bSuccess, Reason);
