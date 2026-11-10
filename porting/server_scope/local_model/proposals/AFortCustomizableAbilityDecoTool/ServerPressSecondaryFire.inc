// Original address: 0x1428ce460.
// CONFIDENCE: high
// WithValidation Server RPC thunk. The reflected exec (flags 0x80220cc0) does exactly:
//   1) virtual call to this->vtable[0xb50] (no args, returns bool) = ServerPressSecondaryFire_Validate
//   2) if false: RPC_ValidateFailed(TEXT("ServerPressSecondaryFire_Validate")) and return
//   3) virtual call to this->vtable[0xb58] (this) = ServerPressSecondaryFire_Implementation
// The validate name is proven by the RPC_ValidateFailed wide-string literal in the body; the
// implementation name is confirmed by the promoted header (AFortCustomizableAbilityDecoTool.h)
// which declares `void ServerPressSecondaryFire_Implementation();`. The two calls sit in
// consecutive vtable slots (0xb50/0xb58), matching the UE <Name>_Validate + <Name>_Implementation
// pairing for a WithValidation Server RPC.
//
// GENERATOR DISCREPANCY: the promoted header emits `UFUNCTION(Server, Reliable)` WITHOUT the
// WithValidation specifier and omits the `bool ServerPressSecondaryFire_Validate()` declaration
// (cf. AFortPlayerControllerAthena.h, which does emit both for its WithValidation RPCs). The
// original RPC is WithValidation (the RPC_ValidateFailed literal proves the _Validate contract),
// so the header must add `WithValidation` and declare `bool ServerPressSecondaryFire_Validate();`
// for this body to compile.
if (!ServerPressSecondaryFire_Validate())
{
	RPC_ValidateFailed(TEXT("ServerPressSecondaryFire_Validate"));
	return;
}
ServerPressSecondaryFire_Implementation();
