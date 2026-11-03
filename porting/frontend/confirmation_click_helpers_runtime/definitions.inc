// Fragments recovered from image-125.runtime.bin (file offset = VA - 0x140000000).
// NOT COMPILABLE AS-IS. Callee names prefixed blk_ are unresolved; see NOTES.md.
// Not built, not run, not committed.

// ---- VA 0x144f5fbc0: confirm-button helper (rcx = window W, rdx = clicked button B) ----
// Verified control flow from disassembly (offsets are window-relative):
void UFortConfirmationWindow::HandleConfirmButtonClicked_Helper(UCommonButton* Button)
{
	// [verified] null B -> return
	if (!Button) return;

	// [verified] call 0x143d66ef0(W); false -> return. Gate, meaning inferred (see blockers).
	if (!blk_WindowGate_3d66ef0(this)) return;

	// [verified] ConfirmationButtons: data ptr at +0x498, ArrayNum at +0x4a0, element stride 0x10,
	// element.Button at +0x0 (FFortConfirmationButtonInfo). Element +0x8 is the matched value.
	void* MatchedValue = nullptr;   // local rsp+0x48, zeroed
	for (const auto& Info : ConfirmationButtons)   // +0x498 ptr, +0x4a0 count
	{
		if (Info.Button == Button)
		{
			MatchedValue = blk_InfoValue_at_plus8(Info);   // [verified offset +0x8, type unknown]
			break;
		}
	}

	// [verified] byte at +0x596 compared to 0:
	if (blk_Byte596 == 0)
	{
		// [verified] call 0x14f55c60(W, rdx=0, &MatchedValue); then return.
		blk_Helper_4f55c60(this, 0, &MatchedValue);
		return;
	}

	// [verified] vtable call on B: slot +0x268 (rcx = B). Return value unused.
	blk_Button_VTable268(Button);

	// [verified] controller = 0x144ade110(W) (cast/owner helper, class-tree check); null -> return.
	auto* Controller = blk_Cast_4ade110(this);
	if (!Controller) return;

	// [verified] call 0x144ce33d0(Controller, dl=0, r8=MatchedValue, r9=&ConfirmationDescription (+0x4b0),
	//   [rsp+0x28]=W+0x490 (unreflected), [rsp+0x20]=1)
	blk_Dispatch_4ce33d0(Controller, /*bool*/ false, MatchedValue,
	                     &ConfirmationDescription, /*+0x490*/ blk_Slot490, /*bool*/ true);

	// [verified] call 0x143d852b0(EntryBox_SimpleButtons (+0x448), dl=1)
	blk_SetVisible_3d852b0(EntryBox_SimpleButtons, true);

	// [verified] tail-jump 0x1407f7e20(CommonLoadGuard_LatentSpinner (+0x488), dl=1)
	blk_Tail_7f7e20(CommonLoadGuard_LatentSpinner, true);
}

// ---- VA 0x144f5fce0: decline-button helper (rcx = window W, rdx ignored) ----
void UFortConfirmationWindow::HandleDeclineButtonClicked_Helper()
{
	// [verified] rdx is not read.
	// [verified] call 0x143d66ef0(W); false -> return (same gate as confirm).
	if (!blk_WindowGate_3d66ef0(this)) return;

	// [verified] copy ConfirmationDescription (+0x4b0) into a local (call 0x144f4cfd0).
	FFortDialogDescription_NUI DescCopy = ConfirmationDescription;   // [inferred: copy ctor]

	// [verified] call 0x144f75f90(W) and call 0x144f55ae0(W): side effects unknown.
	blk_Step_4f75f90(this);
	blk_Step_4f55ae0(this);

	// [verified] controller = 0x144ade110(W); null -> cleanup and return.
	auto* Controller = blk_Cast_4ade110(this);
	if (!Controller) return;

	// [verified] local 0x170 built by 0x141f7ce20; r8 taken from local 0x178 after that call.
	blk_Local170 Local170;
	void* R8 = blk_Local170.Second;   // [verified read location, meaning unknown]

	// [verified] call 0x144ce33d0(Controller, dl=1, r8=R8, r9=&DescCopy,
	//   [rsp+0x28]=&Local170, [rsp+0x20]=0)
	blk_Dispatch_4ce33d0(Controller, /*bool*/ true, R8, &DescCopy, &Local170, /*bool*/ false);
	// [verified] cleanup: 0x144ae20d0(DescCopy); return.
}
