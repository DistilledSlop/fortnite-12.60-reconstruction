// CONFIDENCE: HIGH. Stores the new defender trap and raises the trap-changed event with the previous trap as its argument.
// Original address: 0x1427a5d00 (exec). Native body: 0x1422ec330 (store at +0x18c0), event dispatch 0x1427a1ee0 (FindFunctionChecked + ProcessEvent).
AActor* OldDefenderTrap = DefenderTrap;
DefenderTrap = InDefenderTrap;
ProcessEvent(FindFunctionChecked(DefenderTrapChangedFunction), &OldDefenderTrap);
