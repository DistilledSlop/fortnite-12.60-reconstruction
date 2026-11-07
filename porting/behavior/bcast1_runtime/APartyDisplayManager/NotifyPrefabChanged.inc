// Behaviour snippet: APartyDisplayManager::NotifyPrefabChanged (wrapper body; spliced into the generated member function).
// Original body address: 0x141fc9350 (thunk 0x142b7e690, receiver delta 0, this delta total 0).
// Offset evidence: RDX (PrefabActor) stored at param offset 0 and R8 (CurrentItem) at offset 8: matches FOnPrefabForCurrentlyDisplayedItemChanged(AActor* NewPrefab, UFortItem* NewItem) in ABI order.
// Offset evidence: ADD RCX,0x278 -> OnPrefabForCurrentlyDisplayedItemChanged (reflected at +0x278).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnPrefabForCurrentlyDisplayedItemChanged.Broadcast(PrefabActor, CurrentItem);
