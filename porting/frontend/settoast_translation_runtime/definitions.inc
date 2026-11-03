// UFortFriendSubscriptionToastWidget::SetToast translation (body VA 0x144d97670).
// Pseudo-C: porting/generated/decomp-imports/Imports/SetToast__144d97670.c. Names: see declarations.inc.
// Compiler stack-cookie prologue/epilogue (0x145803c70 check) is not translated.

void UFortFriendSubscriptionToastWidget::SetToast(UFortUINotification* ToastNotification)
{
	// S1: this+0x280 = notif (verified: mov %rdx,0x280(%rcx)).
	NativeToastNotification = ToastNotification;

	// S2: name lookup on this+0x2c0; result unused.
	uint64 NameStorageA = 0;
	uint64* NameA = NativeFNameFromLiteral142cd49c0(&NameStorageA, "SubscriptionNudgeHold", 1);
	NativeMapFindByName144ddd7d0(NativeMap2C0, *NameA);

	// S3: bHasAction candidate at notif+0x58 (byte). If zero, go to the common tail (S12).
	if (NativeFieldAt<uint8>(ToastNotification, 0x58) != 0)
	{
		// S4: action branch. Receiver is this (vcall +0x288).
		void* Rbx = NativeSlot288(this);
		if (Rbx != nullptr)
		{
			// S5: no argument is set up at the call site (verified). Return object used in a membership guard.
			void* Lookup = NativeGetObject142a53450();
			void* Holder = NativeFieldAt<void*>(Rbx, 0x10);
			int32 LookupCount = NativeFieldAt<int32>(Lookup, 0x38);
			int32 HolderCount = NativeFieldAt<int32>(Holder, 0x38);
			void** HolderData = reinterpret_cast<void**>(NativeFieldAt<void*>(Holder, 0x30));
			if (LookupCount <= HolderCount && HolderData[LookupCount] == static_cast<uint8*>(Lookup) + 0x30)
			{
				// S6: predicate selects the sub-object. True -> this+0x2a8, false -> this+0x2b0 (verified).
				void* Selected = NativePredicate14238ead0(Rbx) ? NativeObject2A8 : NativeObject2B0;

				// S7: receiver this+0x2b8, slot +0x2e8 (verified).
				NativeSlot2E8(NativeObject2B8, Selected);

				// S8: sub-object at rbx+0x3138. Null -> S10 (verified).
				void* SubObject = NativeFieldAt<void*>(Rbx, 0x3138);
				if (SubObject != nullptr)
				{
					// S9: bind only if the predicate is false (verified: call 0x1440f3f60 precedes the binds).
					if (!NativePredicate1440f3f60(SubObject))
					{
						// Hold-pressed bind: IE 0, delegate Func = 0x144d80480 (verified).
						NativeBindDelegate16 HoldPressed;
						HoldPressed.Func = reinterpret_cast<void*>(&NativeFriendHoldPressed144d80480);
						HoldPressed.Tail8 = 0;
						uint64 NameStorageB = 0;
						uint64* NameB = NativeFNameFromLiteral142cd49c0(&NameStorageB, "SubscriptionNudgeHold", 1);
						NativeBindAction140848f70(SubObject, *NameB, 0, this, &HoldPressed);

						// Hold-released bind: IE 1, delegate Func = 0x144d806b0 (verified).
						NativeBindDelegate16 HoldReleased;
						HoldReleased.Func = reinterpret_cast<void*>(&NativeFriendHoldReleased144d806b0);
						HoldReleased.Tail8 = 0;
						uint64 NameStorageC = 0;
						uint64* NameC = NativeFNameFromLiteral142cd49c0(&NameStorageC, "SubscriptionNudgeHold", 1);
						NativeBindAction140848f70(SubObject, *NameC, 1, this, &HoldReleased);
					}
					// Receiver rbx, slot +0xbd8 (verified).
					NativeSlotBD8(Rbx, SubObject);
				}
			}
		}

		// S10 (MCP block): requires notif+0xd0 != 0 and notif+0xe0 != 0 (verified).
		if (NativeFieldAt<void*>(ToastNotification, 0xd0) != nullptr &&
		    NativeFieldAt<void*>(ToastNotification, 0xe0) != nullptr)
		{
			uint64 NameStorageMcp = 0;
			void* McpName = NativeFNameFromWideLiteral142cd4ab0(&NameStorageMcp, L"MCP", 1);
			NativeSlot150(this); // result discarded (verified)
			void* Source = NativeGetPtr140d0e9e0(McpName);
			if (Source != nullptr)
			{
				void* McpOut = nullptr;
				NativeGetOut140d126c0(Source, &McpOut);
				if (McpOut != nullptr)
				{
					// Payload: alloc 0x20, vtable 146d714e0, UniqueId at +0x10, vtable 146d71538 (verified).
					// The count 2 is a stack local (movl $0x2,0x48(%rsp)), not a payload field (verified).
					NativeMcpPayloadRef PayloadRef;
					NativeMcpSubPayload* Payload = static_cast<NativeMcpSubPayload*>(NativeAlloc142bfd200(nullptr, 0x20, 0));
					PayloadRef.Payload = Payload;
					PayloadRef.Count = 2;
					if (Payload != nullptr)
					{
						Payload->Vtable = NativeVtbl146d714e0;
						Payload->UniqueId = NativeNextId142bad740();
						Payload->Vtable = NativeVtbl146d71538;
					}
					NativeMcpFriendSub140cd1e80(McpOut,
					                            NativeFieldAt<void*>(ToastNotification, 0xd0),
					                            NativeFieldAt<void*>(ToastNotification, 0xe0),
					                            &PayloadRef);
				}
				// UNKNOWN: a release of the 8-byte pair at stack +0x120 (shared-pointer pattern, lock xadd on +0x8
				// then +0xc) follows here. Its writer is not traced on this path (pseudo-C sets it only on the bind
				// path, to 0). Not translated; see NOTES.md.
			}
		}
	}

	// S12 (common tail): pair {notif+0x40, notif+0x48} with a ref increment, plus int32 notif+0x50 (verified).
	NativeNotifPair2C8 Pair;
	Pair.Obj40 = NativeFieldAt<void*>(ToastNotification, 0x40);
	Pair.Ref48 = NativeFieldAt<void*>(ToastNotification, 0x48);
	if (Pair.Ref48 != nullptr)
	{
		// lock incl on +0x8 of the holder (verified).
		InterlockedIncrement(&NativeFieldAt<int32>(Pair.Ref48, 0x8));
	}
	Pair.Int50 = NativeFieldAt<int32>(ToastNotification, 0x50);

	// S13: receiver this+0x2a0, slot +0x2c8 (verified).
	NativeSlot2C8(NativeObject2A0, &Pair);

	// S14: receiver this+0x2b8, slot +0x270, argument 1, or 4 when bHasAction (verified: cmov on notif+0x58).
	uint8 VisibilityArg = 1;
	if (NativeFieldAt<uint8>(ToastNotification, 0x58) != 0)
	{
		VisibilityArg = 4;
	}
	NativeSlot270(NativeObject2B8, VisibilityArg);

	// S15: receivers this+0x2a8 and this+0x2b0, slot +0x268, argument bHasAction (verified).
	uint8 HasAction = NativeFieldAt<uint8>(ToastNotification, 0x58);
	NativeSlot268(NativeObject2A8, HasAction);
	NativeSlot268(NativeObject2B0, HasAction);

	// S16: owner W = vcall(this, +0x150). Null skips the timer block and goes to the end (verified).
	void* W = NativeSlot150(this);
	if (W != nullptr)
	{
		// S17: uRate from this+0x294, timer owner selection (inferred timer-manager lookup; not verified).
		// If W+0x188 is null, owner = W+0x320; else owner = (W+0x188)+0xc0.
		uint64 TimerOwner = (NativeFieldAt<void*>(W, 0x188) == nullptr)
			? NativeFieldAt<uint64>(W, 0x320)
			: NativeFieldAt<uint64>(NativeFieldAt<void*>(W, 0x188), 0xc0);
		// Float bits at this+0x294 (NativeFloat294). Passed as a 32-bit value; register class (GPR vs XMM) UNKNOWN.
		uint32 RateBits = NativeFieldAt<uint32>(this, 0x294);

		NativeDisplayTimerPayload* Timer =
			static_cast<NativeDisplayTimerPayload*>(NativeAlloc142bfd200(nullptr, 0x30, 0));
		if (Timer != nullptr)
		{
			Timer->Vtable = NativeVtbl1459a9e80;
			NativeWriteWeakRef142ee3b30(&Timer->WeakOwner, this);
			Timer->Callback = reinterpret_cast<void*>(&NativeFriendDisplayTimer144d7fd10);
			Timer->Zero18 = 0;
			// +0x1c: not written (UNKNOWN, see declarations.inc).
			Timer->UniqueId = NativeNextId142bad740();
			Timer->Vtable = NativeVtbl1459a9ed8;
		}

		// Delegate out-object from the timer payload (slot +0x40, verified receiver shape).
		void* DelegatePtr = nullptr;
		if (Timer != nullptr)
		{
			NativeSlot40(Timer, &DelegatePtr);
		}

		uint64 WeakTemp = 0;
		NativeWriteWeakRef142ee3b30(&WeakTemp, nullptr);

		// SetTimer(owner, this+0x270 handle, &delegate, rate, bLoop = 0, FirstDelay = -1.0).
		// Stack-argument order (byte 0, float -1.0) is not verified; see declarations.inc.
		NativeSetTimer1446278d0(reinterpret_cast<void*>(TimerOwner), &NativeTimerHandle270, &DelegatePtr,
		                        RateBits, 0, -1.0f);

		// UNKNOWN: the lStack_88 temp release (vtable +0x10 on plStack_78 or alStack_68) is not translated;
		// the receiver type is not recovered.
		// Delegate release: the pseudo-C guard is iStack_a0 == 0 (never written after its initial 0, because the
		// slot +0x40 call only receives &plStack_a8). On that path, release DelegatePtr if non-null.
		if (DelegatePtr != nullptr)
		{
			NativeRelease142bee710(DelegatePtr);
		}

		// Timer payload cleanup: vtable +0x38 with argument 0, alloc(ptr,0,0) as release, then null-checked release.
		if (Timer != nullptr)
		{
			NativeSlot38(Timer, 0);
			void* Freed = NativeAlloc142bfd200(Timer, 0, 0);
			if (Freed != nullptr)
			{
				NativeRelease142bee710(Freed);
			}
		}
	}

	// S19: tail call with this (semantics UNKNOWN).
	NativeFinish145238060(this);
}
