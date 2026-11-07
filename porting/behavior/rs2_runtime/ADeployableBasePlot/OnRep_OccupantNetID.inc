// Original body address: 0x141c45150 (thunk 0x14277d4c0)
// Offset evidence: CALL 0x141c53230 is ADeployableBasePlot::OnRep_BuildableSpaceExpansionVector on this.
// Delegate member at +0x860 is OnOccupantIdChanged (FOnOccupantIdChanged, one ADeployableBasePlot* param); the packed param is this (RBX).
// CONFIDENCE: HIGH
OnRep_BuildableSpaceExpansionVector();
OnOccupantIdChanged.Broadcast(this);
