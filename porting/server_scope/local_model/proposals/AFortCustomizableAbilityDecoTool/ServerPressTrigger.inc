// Original address: 0x1428ce4b0.
// CONFIDENCE: high
// WithValidation Server RPC thunk. The reflected exec (flags 0x84220cc0) does exactly:
//   1) virtual call to this->vtable[0xb80] (no args, returns bool) = ServerPressTrigger_Validate
//   2) if false: RPC_ValidateFailed(TEXT("ServerPressTrigger_Validate")) and return
//   3) virtual call to this->vtable[0xb88] (this) = ServerPressTrigger_Implementation
// The validate name is proven by the RPC_ValidateFailed wide-string literal in the body; the
// implementation name is confirmed by the promoted header (AFortCustomizableAbilityDecoTool.h)
// which declares `void ServerPressTrigger_Implementation();`. The two calls sit in consecutive
// vtable slots (0xb80/0xb88), matching the UE <Name>_Validate + <Name>_Implementation pairing
// for a WithValidation Server RPC.
//
// GENERATOR DISCREPANCY: the promoted header emits `UFUNCTION(Server, Reliable)` WITHOUT the
// WithValidation specifier and omits the `bool ServerPressTrigger_Validate()` declaration
// (cf. AFortPlayerControllerAthena.h, which does emit both for its WithValidation RPCs). The
// original RPC is WithValidation (the RPC_ValidateFailed literal proves the _Validate contract),
// so the header must add `WithValidation` and declare `bool ServerPressTrigger_Validate();`
// for this body to compile.
if (!ServerPressTrigger_Validate())
{
	RPC_ValidateFailed(TEXT("ServerPressTrigger_Validate"));
	return;
}
ServerPressTrigger_Implementation();
