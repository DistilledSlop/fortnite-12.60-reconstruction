// Helper 0x1423a1ea0 (AFortPlayerControllerFrontEnd host). Callers EmotePickerNextWheel (0x1425ed2c0, Rank=1) and
// EmotePickerPrevWheel (0x1425ed2d0, Rank=-1) tail-jump here. If the controller flag byte at +0x1cb4 has bit 0x40,
// the asm tail-jumps to 0x140f2c470 with RCX = Controller+0x988 and EDX = Rank; otherwise it returns.
// Every instruction of the asm is accounted for.
// CONFIDENCE: HIGH
if ((*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(Controller) + 0x1cb4) & 0x40) != 0)
{
    reinterpret_cast<UFortControllerComponent_MinigameActivity*>(reinterpret_cast<uint8*>(Controller) + 0x988)->ForwardMinigameActivityRankChanged(Rank);
}
