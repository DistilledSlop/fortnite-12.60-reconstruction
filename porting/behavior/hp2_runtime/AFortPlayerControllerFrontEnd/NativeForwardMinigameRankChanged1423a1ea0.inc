// Original body address: 0x1423a1ea0 (helper; tail-jumps to 0x140f2c470 with RCX = this+0x988)
// Offset evidence: byte +0x1cb4 mask 0x40 is bUsePickers (AFortPlayerController bitfield at offset 7348 in bool_bits.json, mask 64). Inline component at +0x988 has no reflected member.
// Asm: if bUsePickers is clear, return; otherwise forward the int32 Rank in EDX to UFortControllerComponent_MinigameActivity::ForwardMinigameActivityRankChanged on the component.
// Callers EmotePickerNextWheel and EmotePickerPrevWheel set EDX to 1 and -1 and tail-jump here.
// CONFIDENCE: MEDIUM
uint8* ControllerBytes = reinterpret_cast<uint8*>(FrontEndController);
if ((ControllerBytes[0x1cb4] & 0x40) == 0)
{
	return;
}
UFortControllerComponent_MinigameActivity* MinigameComponent = reinterpret_cast<UFortControllerComponent_MinigameActivity*>(ControllerBytes + 0x988);
MinigameComponent->ForwardMinigameActivityRankChanged(Rank);
