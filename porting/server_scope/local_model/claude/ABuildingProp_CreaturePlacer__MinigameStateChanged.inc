// CONFIDENCE: LOW. Forwards the minigame state change to the placer's native handler (virtual slot 0xf08, native body 0x141c10360), which is the real behaviour.
// Original address: 0x14274b420. Parameter order: the thunk reads the object first and the one-byte state second.
NativeMinigameStateChanged(Minigame, NewMinigameState);
