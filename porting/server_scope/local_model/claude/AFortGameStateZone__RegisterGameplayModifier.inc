// CONFIDENCE: MEDIUM. Exec thunk 0x14292c940 reads InModifierToRegister (object) and Expiration (int32), calls RegisterGameplayModifier_Implementation (0x141ea8c40) and returns its handle.
// Original address: 0x14292c940 (exec). Helpers: RegisterGameplayModifier_Implementation (deps AFortGameStateZone__RegisterGameplayModifier_Implementation).
return RegisterGameplayModifier_Implementation(InModifierToRegister, Expiration);
