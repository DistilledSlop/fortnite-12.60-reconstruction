// Helper 0x142f24280: inlined FPlatformApplicationMisc::ClipboardCopy(const TCHAR*). IAT calls resolved with
// porting/pe_imports.py: GetActiveWindow 0x1458fd3f8, OpenClipboard 0x1458fd3b8, EmptyClipboard 0x1458fd528,
// GlobalAlloc(GMEM_MOVEABLE=2, (len+1)*2) 0x1458fc608, GlobalLock 0x1458fc618, wide strcpy loop, GlobalUnlock 0x1458fcad0,
// SetClipboardData(CF_UNICODETEXT=0xd) 0x1458fd3c8, CloseClipboard 0x1458fd3c0.
// The failure branch (GetLastError 0x1458fc8e8 + logging helpers 0x141b6ad20/0x142f1d890/0x142c7b670) is a verbosity-gated
// error log and is elided; the platform ClipboardCopy reports the same failure itself.
// Callers (HandleCopyCode 0x144d7c9a0): RCX = FString data pointer ([this+0x4e0]) or a literal; argument [RSP+0x90] is the TCHAR*.
FPlatformApplicationMisc::ClipboardCopy(TextToCopy);
// CONFIDENCE: MEDIUM
