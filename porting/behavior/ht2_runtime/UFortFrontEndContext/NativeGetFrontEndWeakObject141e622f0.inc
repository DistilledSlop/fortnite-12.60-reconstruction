// Helper 0x141e622f0, host UFortFrontEndContext (static). 2 instructions: ADD RCX,0x468; JMP 0x142ee5dd0.
// 0x142ee5dd0 = TWeakObjectPtr::Get, tail-called with this = context + 0x468 (no reflected member at 0x468; lookup_offset.py UFortFrontEndContext 0x468).
// Callers: AFortCharmPreviewActor::ApplyMyCosmeticItemToHero (0x141da06f0), UFortActivatablePanelWithItemPreview::OnCosmeticSpecialActionPressed (0x144b884f0).
// CONFIDENCE: HIGH
return reinterpret_cast<const TWeakObjectPtr<UObject>*>(reinterpret_cast<const uint8*>(FrontEndContext) + 0x468)->Get();
