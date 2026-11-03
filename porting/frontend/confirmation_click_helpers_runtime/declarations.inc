// Declarations for the two helpers at VA 0x144f5fbc0 (confirm) and 0x144f5fce0 (decline).
// Intended for UFortConfirmationWindow private section. Not placed; not built.
// Existing reflected members already in porting/native/Source/FortniteUI/Generated/UFortConfirmationWindow.h:
//   ConfirmationButtons (TArray<FFortConfirmationButtonInfo>, +0x498 ptr, +0x4a0 count),
//   DeclineButton (UCommonButton*, +0x4a8), ConfirmationDescription (FFortDialogDescription_NUI, +0x4b0),
//   EntryBox_SimpleButtons (+0x448), CommonLoadGuard_LatentSpinner (+0x488).

// Confirm-button helper (VA 0x144f5fbc0)
void HandleConfirmButtonClicked_Helper(UCommonButton* Button);

// Decline-button helper (VA 0x144f5fce0). Second argument is ignored by the callee.
void HandleDeclineButtonClicked_Helper();

// Unresolved byte/slot members used by the bodies above (blockers, do not add to the generated header yet):
//   uint8 Byte596;   // window +0x596, owner class unknown, compared to 0 in confirm helper
//   void* Slot490;   // window +0x490, unreflected 8-byte slot, passed to 0x144ce33d0 as arg 5

// Blocked helper symbols (unresolved; need real owners before use):
//   blk_WindowGate_3d66ef0, blk_Cast_4ade110, blk_Dispatch_4ce33d0, blk_Helper_4f55c60,
//   blk_Button_VTable268, blk_SetVisible_3d852b0, blk_Tail_7f7e20, blk_Step_4f75f90,
//   blk_Step_4f55ae0, blk_Local170
