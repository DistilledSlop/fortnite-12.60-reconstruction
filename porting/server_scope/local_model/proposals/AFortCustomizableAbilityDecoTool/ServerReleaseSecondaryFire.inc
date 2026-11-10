// Original address: 0x1428ce500.
// CONFIDENCE: high
// Standard native server-RPC body: run the validation hook, abort with
// RPC_ValidateFailed on failure, otherwise dispatch the implementation.
// Decompile (captures/078403b6137a48d6a440db6fd38ebad7/0x1428ce500.{c,asm}) shows,
// after the FFrame +0x20 guard-increment prologue:
//   CALL [vtable + 0xb38]  -> ServerReleaseSecondaryFire_Validate()  (bool)
//   if (!result) { RPC_ValidateFailed(L"ServerReleaseSecondaryFire_Validate"); return; }
//   JMP  [vtable + 0xb40]  -> ServerReleaseSecondaryFire_Implementation()
// The validate name is proven by the wide string at 0x1463b01c0 ("ServerReleaseSecondaryFire_Validate")
// loaded into RCX and passed to RPC_ValidateFailed (0x142d6e0f0) on the failure path.
// Both the validate and implementation hooks are virtual (vtable 0xb38 / 0xb40) and are
// preserved as such; the dispatch is not inlined.
*(uint8**)((uint8*)&Stack + 0x20) += (*(uint8**)((uint8*)&Stack + 0x20) != nullptr);
if (!ServerReleaseSecondaryFire_Validate())
{
	RPC_ValidateFailed(TEXT("ServerReleaseSecondaryFire_Validate"));
	return;
}
ServerReleaseSecondaryFire_Implementation();
