// Behaviour snippet: AFortCreativeMoveTool::OnRep_SelectionProperty (wrapper body; spliced into the generated member function).
// Original body address: 0x14267d110 (thunk 0x1428c3920, receiver delta 0, this delta total 0).
// Offset evidence: MOVZX EAX,byte [RCX+0xf32] (reflected SelectionProperty, ESelectionProperty : uint8) stored as a 1-byte param at offset 0.
// Offset evidence: ADD RCX,0xd68 -> OnSelectionPropertyChanged (reflected at +0xd68; FOnSelectionPropertyChanged(ESelectionProperty)).
// Callee 0x1406a9dc0 = TMulticastScriptDelegate::ProcessMulticastDelegate<UObject>; called as the shared multicast broadcast body (no callee_candidates for this job).
// CONFIDENCE: HIGH
OnSelectionPropertyChanged.Broadcast(SelectionProperty);
