// CONFIDENCE: HIGH. Exec thunk 0x14292c470 reads WaveData, LevelText, Level, WaveText and WaveNum in offset order, then calls the virtual OnWaveEncounterStarted_Implementation (vtable +0xa00 resolves to 0x141ea72e0, deps AFortGameStateZone__OnWaveEncounterStarted_Implementation).
// Original address: 0x14292c470 (exec). WaveData is read but not used by the implementation.
OnWaveEncounterStarted_Implementation(WaveData, LevelText, Level, WaveText, WaveNum);
