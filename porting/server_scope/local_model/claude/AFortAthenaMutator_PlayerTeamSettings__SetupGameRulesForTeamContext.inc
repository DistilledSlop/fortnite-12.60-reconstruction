// CONFIDENCE: LOW. Original address: 0x142815310 (exec thunk). The body is a virtual dispatch on this at vslot 0xcd8.
// Recovered evidence: slot 0xcd8 of the PlayerTeamSettings primary vtable is the first entry after its 411 measured
// slots and resolves to 0x141af2d68, a this-adjusting thunk (this - 0x218) into 0x141af35b0, which is a destructor-shaped
// body (returns this). So the native class does not supply a SetupGameRules body at that slot; the call is left to the
// overriding implementation (Blueprint or derived class). Native fallback result: false.
return SetupGameRulesForTeamContext_Implementation(Minigame);
