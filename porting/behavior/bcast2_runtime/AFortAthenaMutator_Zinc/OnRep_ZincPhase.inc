// Behaviour snippet: AFortAthenaMutator_Zinc::OnRep_ZincPhase (body only; spliced into the generated member function).
// Original body: 0x141b4ff40 (thunk 0x14282e910); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: MOVZX EAX,byte [RCX+0x320] (ZincPhase, reflected Enum); slot0 = this, slot8 = AL.
//   ADD RCX,0x310 -> OnZincPhaseChanged (+0x310), FZincPhaseChanged(AFortAthenaMutator_Zinc* Mutator, EZincPhase Phase).
// CONFIDENCE: HIGH
OnZincPhaseChanged.Broadcast(this, ZincPhase);
