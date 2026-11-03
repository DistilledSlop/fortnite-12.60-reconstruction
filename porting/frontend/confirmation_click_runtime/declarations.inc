// Class-body declarations for two recovered click handlers.
// Integrator places each block in the named class body. Raw offsets are from
// /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin (VA - 0x140000000).

// ---- UFortConfirmationWindow (private section) ----
// Reflected exec 145218540 (flags 0x04080401, no params) tail-jumps to body 144f54030.
// Replaces the BotClickButton entry in confirmation_window_lifecycle_runtime/members.inc.
// Do not keep NativeBotControllerEligible144f54030 as a separate helper: 144f54030 is this body.
UFUNCTION(BlueprintCallable)
void BotClickButton();

// Raw button pointer at window +0x4a8, passed as the argument of 144f5fce0 on the
// decline branch. Writer not recovered; the callee ignores the argument.
UCommonButton* NativeDeclineButtonPtr4A8 = nullptr;

// Window +0x570 (int32) is compared with > 0 before ConfirmationButtons is consulted.
// Inferred to be ConfirmationDescription.ConfirmButtonInputActions.Num(); offset not yet mapped.
// Declared only as a raw count accessor so the integrator can bind it to the real field.
int32 NativeConfirmButtonInputActionCount570() const;

// ---- UFortRestartClientModal (public/protected section) ----
// Reflected exec 1452c2a80 (flags 0x04080401, no params) tail-jumps to body 144f5f9f0.
UFUNCTION(BlueprintCallable)
void HandleButtonClicked();

// Byte +0x448. Written by setter 144f7b54f (second argument). Initialized to 0 by ctor 145228430.
bool bNativeRestartFlag448 = false;
// Byte +0x304. Activation-stack membership flag (see restart_modal_click_runtime/HANDOFF.md).
bool bNativeStackMembership304 = false;
// Byte +0x288. Read through the fallback helper 1407b7ed0. Owning class layout not confirmed
// (may belong to the UFortActivatablePanel/UCommonActivatableWidget base).
bool bNativeFlag288 = false;

// BLOCKED virtual dispatch on the modal vtable at +0x4c0. The shipping target is 1407e1130
// (panel pop through the weak manager at +0x3b0). Must bind to the existing base override slot,
// not a new virtual.
void NativeDispatchSlot4C0();

// BLOCKED virtual dispatch on the modal vtable at +0x4a8. Target not recovered.
void NativeDispatchSlot4A8();

// BLOCKED helper: 141ed81a0 takes (bool, byte) and is the restart platform call. Body not recovered.
void NativeRestartPlatformRequest141ed81a0(bool bFirst, bool bShouldExit);
