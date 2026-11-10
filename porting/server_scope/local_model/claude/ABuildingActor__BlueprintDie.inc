// CONFIDENCE: MEDIUM. Exec thunk: reads DeathReason, Killer, DamageCauser (declaration order) and forwards them to the native death path. The wrapper adds no logic of its own.
// Original address: 0x142736450 (exec). Native target: 0x141b94c30 via virtual slot 0x860 (see deps ABuildingActor__NativeDie).
NativeDie(DeathReason, Killer, DamageCauser, nullptr);
