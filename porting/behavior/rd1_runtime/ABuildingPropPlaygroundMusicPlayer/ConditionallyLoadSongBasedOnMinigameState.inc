// Original body address: 0x141b3d600 (thunk 0x14274fe00). Body asm: porting/generated/native-cap-tailjmp0/0x141b3d600.asm
// Offset evidence: [RCX+0xc08] = MinigameLogicComponent (reflected Object); [RCX+0xc20/0xc24/0xc28/0xc2c/0xc30] = PlayDuringPregame/Warmup/Gameplay/RoundEnd/GameEnd (reflected Int).
// Pointee offsets (unreflected, raw): [logic+0x150] = state object, [state+0x870] = phase byte.
// Phase 0 -> Pregame, 4 -> Warmup, 5 -> Gameplay, 7 -> RoundEnd (calls OnRep first when set) then GameEnd, 8 -> GameEnd, other -> return. Tail call to OnRep_PlayingSongData when the flag is set.
// CONFIDENCE: HIGH
const uint8* LogicRaw = reinterpret_cast<const uint8*>(MinigameLogicComponent);
const uint8* StateObject = *reinterpret_cast<const uint8* const*>(LogicRaw + 0x150);
if (StateObject == nullptr) { return; }
const uint8 Phase = StateObject[0x870];
bool bPlay = false;
switch (Phase)
{
    case 0: bPlay = PlayDuringPregame != 0; break;
    case 4: bPlay = PlayDuringWarmup != 0; break;
    case 5: bPlay = PlayDuringGameplay != 0; break;
    case 7:
        if (PlayDuringRoundEnd != 0) { OnRep_PlayingSongData(); }
        bPlay = PlayDuringGameEnd != 0;
        break;
    case 8: bPlay = PlayDuringGameEnd != 0; break;
    default: return;
}
if (bPlay) { OnRep_PlayingSongData(); }
