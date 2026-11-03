// Bodies for the confirmation-window bot-click chain, translated from the Ghidra pseudo-C.
// Source files: BotClick__144f54030, ConfirmClicked__144f5fbc0, DeclineClicked__144f5fce0,
// Dispatch__144ce33d0, Present__144f55c60, WeakPin__143d66ef0, OuterWalk__144ade110, u2__144f75f90, u3__144f55ae0.
// Not built, not run. Raw offsets are kept where the pseudo-C gives no field name.

// ---- 0x144f54030: UFortConfirmationWindow::BotClickButton (reflected) ----
void UFortConfirmationWindow::BotClickButton()
{
	void* Obj = NativeDispatchSlot288(this);              // vtable 0x288 on this
	if (!Obj) return;

	void* Cls = NativeClassGetter142a3d9f0();
	if (!NativeClassIsChildOf144ade110(Obj, Cls)) return;

	// Obj+0x1838 and Obj+0x1818 are byte flags (UNKNOWN owner). bFlag = either is nonzero.
	const bool bFlag = (*(const char*)((const uint8*)Obj + 0x1838) != 0) ||
	                   (*(const char*)((const uint8*)Obj + 0x1818) != 0);
	if (!bFlag) return;

	// ConfirmationDescription.ConfirmButtonInputActions.ArrayNum is window +0x570 (LAYOUT.md, inferred +8 ArrayNum).
	if (ConfirmationDescription.ConfirmButtonInputActions.Num() > 0 && ConfirmationButtons.Num() > 0)
	{
		NativeConfirmClicked144f5fbc0(this, ConfirmationButtons[0].Button);   // pseudo: *(undefined8*)data
	}
	else
	{
		NativeDeclineClicked144f5fce0(this, DeclineButton);
	}
}

// ---- 0x144f5fbc0: confirm-button helper ----
void NativeConfirmClicked144f5fbc0(UFortConfirmationWindow* W, UCommonButton* Button)
{
	if (!Button) return;
	if (!NativeWeakPin143d66ef0(W)) return;

	uint64 Value = 0;                                     // pseudo: alStackX_10[0] = 0
	for (int32 i = 0; i < W->ConfirmationButtons.Num(); ++i)
	{
		FFortConfirmationButtonInfo& Info = W->ConfirmationButtons[i];
		if (Info.Button == Button)
		{
			// Element +0x8, 8 bytes, type UNKNOWN.
			Value = *reinterpret_cast<const uint64*>(reinterpret_cast<const uint8*>(&Info) + 0x8);
			break;
		}
	}

	if (W->NativeFlag596 == 0)
	{
		NativePresent144f55c60(W, false, &Value);
		return;
	}

	NativeDispatchSlot268(Button);                        // vtable 0x268 on Button, result unused
	void* Controller = NativeOuterWalk144ade110(W);
	if (!Controller) return;

	NativeDispatch144ce33d0(Controller, false, Value, reinterpret_cast<uint8*>(&W->ConfirmationDescription),
	                        true, reinterpret_cast<uint32*>(W->NativeMember490));
	NativeFunc143d852b0(W->EntryBox_SimpleButtons, true);  // pseudo: thunk_FUN_143d853b0; disasm call target 0x143d852b0
	NativeFunc1407f7e20(W->CommonLoadGuard_LatentSpinner, true);
}

// ---- 0x144f5fce0: decline helper ----
void NativeDeclineClicked144f5fce0(UFortConfirmationWindow* W, UCommonButton* UnusedDeclineButton)
{
	(void)UnusedDeclineButton;                            // pseudo-C never reads the second argument
	uint64 Zero = 0;                                      // pseudo: uStackX_20 = 0
	if (!NativeWeakPin143d66ef0(W)) return;

	uint8 DescCopy[288];                                  // pseudo: auStack_128 [288] (sizeof UNVERIFIED)
	NativeCopyDescription144f4cfd0(DescCopy, reinterpret_cast<const uint8*>(&W->ConfirmationDescription));
	NativeHelper144f75f90(W);
	NativeHelper144f55ae0(W);

	void* Controller = NativeOuterWalk144ade110(W);
	if (Controller)
	{
		uint32 HandleBuf[2];                              // pseudo: auStackX_18 [8]
		NativeInit141f7ce20(HandleBuf);
		NativeDispatch144ce33d0(Controller, true, Zero, DescCopy, false, HandleBuf);
	}
	NativeDestroyDescription144ae20d0(DescCopy);
}

// ---- 0x144f55c60: present wrapper ----
void NativePresent144f55c60(UFortConfirmationWindow* W, bool bFlag, const uint64* ValuePtr)
{
	if (!NativeWeakPin143d66ef0(W)) return;

	uint8 DescCopy[288];
	NativeCopyDescription144f4cfd0(DescCopy, reinterpret_cast<const uint8*>(&W->ConfirmationDescription));
	NativeHelper144f75f90(W);
	NativeHelper144f55ae0(W);

	void* Controller = NativeOuterWalk144ade110(W);
	if (Controller)
	{
		uint32 HandleBuf[2];
		NativeInit141f7ce20(HandleBuf);
		NativeDispatch144ce33d0(Controller, bFlag, *ValuePtr, DescCopy, false, HandleBuf);
	}
	NativeDestroyDescription144ae20d0(DescCopy);
}

// ---- 0x143d66ef0: gate (pseudo-C shape; pinned-object type UNKNOWN) ----
bool NativeWeakPin143d66ef0(UFortConfirmationWindow* W)
{
	// Out pair written by NativeWeakPinOut143d5a140 (UNKNOWN, body not in set): {object ptr, shared-ref block}.
	struct { void* Obj; void* Block; } Pinned = { nullptr, nullptr };
	NativeWeakPinOut143d5a140(W, &Pinned);

	bool bResult = false;
	if (Pinned.Obj != nullptr)
	{
		uint8* Obj = reinterpret_cast<uint8*>(Pinned.Obj);
		if (*(int32*)(Obj + 0x1b8) != 0 && *(void**)(Obj + 0x1b0) != nullptr)
		{
			void* P = *(void**)(Obj + 0x1b0);
			if (NativeQuerySlot28_1b0(P))                 // vtable 0x28 bool query
			{
				uint8 Out = 0;
				NativeWriteSlot48_1b0(P, &Out);           // vtable 0x48 writes one byte
				*(uint8*)(Obj + 0x1a8) = Out;
			}
		}
		bResult = (*(uint8*)(Obj + 0x1a8) & 0x19) != 0;
	}
	// Release of the shared-ref block (pseudo-C tail). Block+0x8 shared count, Block+0xc weak count.
	if (Pinned.Block != nullptr)
	{
		int32* Shared = reinterpret_cast<int32*>(reinterpret_cast<uint8*>(Pinned.Block) + 0x8);
		*Shared -= 1;
		if (*Shared == 0)
		{
			NativeBlockSlot0(Pinned.Block);                     // vtable slot 0 on block (UNKNOWN)
			int32* Weak = reinterpret_cast<int32*>(reinterpret_cast<uint8*>(Pinned.Block) + 0xc);
			*Weak -= 1;
			if (*Weak == 0)
			{
				NativeBlockSlot8Delete(Pinned.Block, true);     // vtable slot 8, delete flag 1 (UNKNOWN)
			}
		}
	}
	return bResult;
}

// ---- 0x144ade110: typed outer walk ----
void* NativeOuterWalk144ade110(void* Start)
{
	if (!Start) return nullptr;

	void* Cls = NativeClassGetter143dbe060();
	void* Cand = Start;
	if (!NativeClassIsChildOf144ade110(Cand, Cls))
	{
		// pseudo: Start+0x20 (Outer), then Outer+0x20 (Outer of Outer); only one step, no loop.
		void* Outer = *(void**)((uint8*)Start + 0x20);
		if (!Outer) return nullptr;
		Cand = *(void**)((uint8*)Outer + 0x20);
		if (!Cand) return nullptr;
		if (!NativeClassIsChildOf144ade110(Cand, Cls)) return nullptr;
	}

	if (NativeGlobal1481134c8 == nullptr)                 // pseudo: _DAT_1481134c8, lazy init
	{
		uint64 T = NativeInit142cbd480();
		NativeGlobal1481134c8 = NativeInit142cc0600(T, NativeFName148382580);   // pseudo: _DAT_148382580
	}
	void* Result = NativeDispatchSlot290(Cand);           // vtable 0x290 on matched object
	return NativeFunc144ade1e0(NativeGlobal1481134c8, Result);
}

// ---- 0x144ce33d0: dispatch to description objects ----
void NativeDispatch144ce33d0(void* Controller, bool bFlag2, uint64 Value, uint8* Desc, bool bFlag5, uint32* Handle)
{
	(void)Controller;                                     // pseudo-C never reads param_1
	NativeHandleOp141f8f0a0(Handle);
	if (bFlag5 && !bFlag2)
	{
		NativeHandleOp141f87900(Handle);
	}

	// Desc+0xf8 object, guarded by int32 Desc+0x100. Disasm confirms receiver is the object (rcx).
	if (*(int32*)(Desc + 0x100) != 0 && *(void**)(Desc + 0xf8) != nullptr)
	{
		if (NativeQuerySlot28_F8(*(void**)(Desc + 0xf8)))
		{
			void* Obj = (*(int32*)(Desc + 0x100) == 0) ? nullptr : *(void**)(Desc + 0xf8);
			NativeDispatchSlot50_F8(Obj, bFlag2, Value);
		}
	}

	const uint32 HandleDword = *Handle;                   // pseudo: uVar1 = *param_6

	// Desc+0x108 object, guarded by int32 Desc+0x110.
	if (*(int32*)(Desc + 0x110) != 0 && *(void**)(Desc + 0x108) != nullptr)
	{
		if (NativeQuerySlot28_108(*(void**)(Desc + 0x108)))
		{
			void* Obj = (*(int32*)(Desc + 0x110) == 0) ? nullptr : *(void**)(Desc + 0x108);
			NativeDispatchSlot50_108(Obj, bFlag2, Value, bFlag5, HandleDword);
		}
	}
}

// ---- 0x144f75f90: u2 ----
void NativeHelper144f75f90(UFortConfirmationWindow* W)
{
	uint8 Tmp[288];
	NativeHelper142b1bea0(Tmp);
	NativeHelper144f4f6a0(reinterpret_cast<uint8*>(&W->ConfirmationDescription), Tmp);
	NativeDestroyDescription144ae20d0(Tmp);
	NativeHelper144f555e0(W);
	NativeHelper144f7b690(W);
	NativeHelper144f7b730(W);

	void* Obj = NativeMember480;
	NativeDispatchSlot2C0(Obj, reinterpret_cast<const uint8*>(&W->ConfirmationDescription));
	NativeDispatchSlot270(Obj, NativeMember4F8 == 0);     // pseudo-C tail jump
}

// ---- 0x144f55ae0: u3 ----
void NativeHelper144f55ae0(UFortConfirmationWindow* W)
{
	NativeHelper144f555e0(W);

	const int32 N = W->NativeEntryArray5C8.Num();
	bool bFound = false;
	for (int32 i = 0; i < N; ++i)
	{
		if (W->NativeEntryArray5C8[i].Count != 0 && W->NativeEntryArray5C8[i].Ptr != nullptr)
		{
			bFound = true;
			break;
		}
	}
	if (!bFound)
	{
		NativeHelper1407b7ed0(W);
		return;
	}

	W->NativeCounter5DC += 1;
	bool bFailed = false;
	for (int32 i = N - 1; i >= 0; --i)                    // pseudo: iterates from ArrayNum-1 down to 0
	{
		const FNativeEntry144f55ae0& E = W->NativeEntryArray5C8[i];
		if (E.Count == 0 || E.Ptr == nullptr || !NativeDispatchSlot50_Entry(E.Ptr, W))
		{
			bFailed = true;
		}
	}
	W->NativeCounter5DC -= 1;

	if (bFailed)
	{
		NativeHelper140685370(&W->NativeEntryArray5C8, false);
	}
}

// ---- Shared class test used by 144f54030 and 144ade110 (raw offsets from pseudo-C) ----
// Obj+0x10 = ClassPrivate. Cls+0x38 = depth, Cls+0x30 = address used as a parent-array entry.
// ObjClass+0x38 = depth, ObjClass+0x30 = parent-array pointer. Field names UNKNOWN.
bool NativeClassIsChildOf144ade110(void* Obj, void* Cls)
{
	const uint8* ObjClass = *(const uint8* const*)((const uint8*)Obj + 0x10);
	const uint8* C = (const uint8*)Cls;
	const int32 Depth = *(const int32*)(C + 0x38);
	if (Depth > *(const int32*)(ObjClass + 0x38)) return false;
	const uint8* const* Parents = *(const uint8* const* const*)(ObjClass + 0x30);
	return Parents[Depth] == C + 0x30;
}
