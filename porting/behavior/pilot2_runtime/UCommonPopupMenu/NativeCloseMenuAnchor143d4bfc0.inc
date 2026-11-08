// Helper 0x143d4bfc0: RCX = UMenuAnchor (UCommonPopupMenu::RequestClose 0x1407f1730 derefs weak OwningMenuAnchor @+0x414).
// lookup_offset UMenuAnchor: OnMenuOpenChanged @0x140..0x150, so [+0x150] is the unreflected TSharedPtr<SMenuAnchor> MyMenuAnchor
// (UMenuAnchor.h declares it right after the delegate). Asm: null-check, then vtbl[0x248](SMenuAnchor, 0, 0, 0)
// = SMenuAnchor::SetIsOpen(false, false, 0). Engine UMenuAnchor::Close() is exactly
// `if (MyMenuAnchor.IsValid()) MyMenuAnchor->SetIsOpen(false, false);`.
AnchorToClose->Close();
// CONFIDENCE: HIGH
