// UFortFriendSubscriptionToastWidget::SetToast translation: declarations.
// Body VA 0x144d97670 (file offset 0x4d97670). Exec stub 0x14523d570 is not traced.
// Reflected SetToast declaration is already in ../strict_toast_imports_runtime/friend-settoast-declaration.inc.
// NativeToastNotification (this+0x280) is already declared in ../strict_toast_imports_runtime/friend-animation-declaration.inc.
// Every name below is a project-local, address-suffixed name. Types marked void* are not recovered.

// ---- UFortFriendSubscriptionToastWidget class-body members (raw offsets, from the body) ----
// this+0x270: FTimerHandle candidate (passed by address to the SetTimer helper). Size and type UNKNOWN.
uint64 NativeTimerHandle270 = 0;
// this+0x294: float passed as the rate argument of the SetTimer helper. Candidate reflected name DisplayTime
// (Config float); offset is not verified against reflection, so the native name is used.
float NativeFloat294 = 0.0f;
// this+0x2a0: receiver of slot 0x2c8. Type UNKNOWN.
void* NativeObject2A0 = nullptr;
// this+0x2a8: receiver of slot 0x268 and a candidate argument of slot 0x2e8. Type UNKNOWN.
void* NativeObject2A8 = nullptr;
// this+0x2b0: receiver of slot 0x268 and a candidate argument of slot 0x2e8. Type UNKNOWN.
void* NativeObject2B0 = nullptr;
// this+0x2b8: receiver of slots 0x2e8 and 0x270. Type UNKNOWN.
void* NativeObject2B8 = nullptr;
// this+0x2c0: map-like container passed to NativeMapFindByName144ddd7d0. Type UNKNOWN.
void* NativeMap2C0 = nullptr;

// ---- Non-reflected native callees (declarations only; bodies are elsewhere) ----
// Literal "SubscriptionNudgeHold" (ASCII, file 0x6d6e568), FindType argument 1.
// Returns a pointer to an 8-byte name value (decompiled as undefined8*); SetToast dereferences it.
uint64* NativeFNameFromLiteral142cd49c0(void* OutStorage, const char* Literal, int32 FindType);
// Literal "MCP" (UTF-16, file 0x5a95918), FindType argument 1. Return type matches the ASCII variant (UNKNOWN).
uint64* NativeFNameFromWideLiteral142cd4ab0(void* OutStorage, const wchar_t* Literal, int32 FindType);
// Lookup by name on the container at this+0x2c0. Return value is discarded by SetToast; return type UNKNOWN.
void NativeMapFindByName144ddd7d0(void* Container, uint64 Name);
// Called with NO argument set up at the call site (rcx is whatever the preceding vcall left). Return UNKNOWN.
void* NativeGetObject142a53450();
// Bool predicate on an object. Selects this+0x2a8 (true) or this+0x2b0 (false).
bool NativePredicate14238ead0(void* Object);
// Bool predicate on a sub-object at rbx+0x3138. True skips the bind step.
bool NativePredicate1440f3f60(void* SubObject);
// Bind helper: (Object, FName, InputEvent, Owner, Delegate block*). Called with IE 0 then IE 1.
void NativeBindAction140848f70(void* Object, uint64 Name, int32 InputEvent, void* Owner, void* DelegateBlock);
// Weak-reference write: (Destination, Object). Object may be null.
void NativeWriteWeakRef142ee3b30(void* Destination, void* Object);
// Heap allocation helper used as alloc(size) and as (ptr, 0, 0) for release. Exact semantics UNKNOWN.
void* NativeAlloc142bfd200(void* Existing, size_t Size, int32 Extra);
// Atomic counter; returns a 64-bit unique id.
uint64 NativeNextId142bad740();
// Null-checked release helper.
void NativeRelease142bee710(void* Object);
// Returns a pointer from the object returned by NativeFNameFromWideLiteral142cd4ab0; null means skip.
void* NativeGetPtr140d0e9e0(void* Source);
// Writes an out-value into *OutSlot. Out slot is the 8-byte value tested for null.
void NativeGetOut140d126c0(void* Source, void* OutSlot);
// Timer helper (named-inferred FTimerManager::SetTimer). Stack-argument order is not verified.
void NativeSetTimer1446278d0(void* TimerManager, void* Handle, void* DelegatePtr, uint32 RateBits,
                             uint8 StackByteArg, float StackFloatArg);
// Tail call at the end of SetToast with this. Semantics UNKNOWN.
void NativeFinish145238060(void* Self);

// ---- Callback function addresses stored in delegate blocks (signatures UNKNOWN) ----
void NativeFriendHoldPressed144d80480();
void NativeFriendHoldReleased144d806b0();
void NativeFriendDisplayTimer144d7fd10();

// ---- Delegate block layout (16 bytes) passed by pointer as the 5th argument of the bind helper ----
// Offset 0: function address. Offset 8: dword written as 0 (r12d). Offset 12: not traced (UNKNOWN).
struct NativeBindDelegate16
{
	void* Func;
	uint32 Tail8 = 0;
	uint32 Tail12Unverified = 0;
};

// ---- Shared pointer pair passed to slot 0x2c8 ----
// Offset 0: notif+0x40 (8 bytes). Offset 8: notif+0x48 (refcount holder, lock incl at +8). Offset 0x10: notif+0x50 (int32).
struct NativeNotifPair2C8
{
	void* Obj40;
	void* Ref48;
	int32 Int50;
};

// ---- Argument wrapper passed by pointer to NativeMcpFriendSub140cd1e80 (4th arg) ----
// Offset 0: payload pointer. Offset 8: int32 value 2 (stack local, not a payload field).
struct NativeMcpPayloadRef
{
	void* Payload;
	int32 Count = 2;
};

// ---- Payload object for the MCP operation (size 0x20). Vtables 0x146d714e0 then 0x146d71538. ----
struct NativeMcpSubPayload
{
	void* Vtable;     // +0x00
	// +0x08: not written by SetToast (verified from disassembly).
	uint64 UniqueId;  // +0x10
};

// ---- Payload object for the display timer (size 0x30). Vtable 0x1459a9e80 then 0x1459a9ed8. ----
struct NativeDisplayTimerPayload
{
	void* Vtable;     // +0x00
	void* WeakOwner;  // +0x08 (weak ref to this, via NativeWriteWeakRef142ee3b30)
	void* Callback;   // +0x10 (NativeFriendDisplayTimer144d7fd10)
	int32 Zero18 = 0; // +0x18
	// +0x1c: not written on the traced path (pseudo-C reads an uninitialized local); UNKNOWN.
	uint64 UniqueId;  // +0x28
};

// ---- Field view helper (raw offset access, no semantics). Used for UFortUINotification offsets ----
// Notif offsets used: +0x40 (Obj), +0x48 (ref holder), +0x50 (int32), +0x58 (byte, bHasAction candidate),
// +0xd0 and +0xe0 (MCP operands). Reflected names exist for some fields; offsets are not verified.
template <typename T>
inline T& NativeFieldAt(void* Base, int32 Offset)
{
	return *reinterpret_cast<T*>(static_cast<uint8*>(Base) + Offset);
}

// ---- Virtual slot stubs (receiver = object at this+X; slot = byte offset in its vtable) ----
void* NativeSlot288(void* Self);                       // this, returns rbx
void* NativeSlot150(void* Self);                       // this, returns owner W
void NativeSlot2E8(void* Self, void* Arg);             // receiver this+0x2b8
void NativeSlot2C8(void* Self, void* Arg);             // receiver this+0x2a0
void NativeSlot270(void* Self, uint8 Arg);             // receiver this+0x2b8; Arg 1 or 4
void NativeSlot268(void* Self, uint8 Arg);             // receivers this+0x2a8 / this+0x2b0
void NativeSlotBD8(void* Self, void* Arg);             // receiver rbx
void NativeSlot40(void* Self, void* OutDelegate);      // receiver display-timer payload
void NativeSlot38(void* Self, uint32 Arg);             // receiver display-timer payload (Arg 0)

// Non-reflected MCP operation (body 0x140cd1e80). Self = out object; returns a bool-like byte (discarded).
uint64 NativeMcpFriendSub140cd1e80(void* Self, void* OperandA, void* OperandB, NativeMcpPayloadRef* PayloadRef);

// ---- Vtable addresses (storage is bound by the integrator; values only) ----
extern void* NativeVtbl146d714e0;   // MCP payload, first vtable write
extern void* NativeVtbl146d71538;   // MCP payload, second vtable write
extern void* NativeVtbl1459a9e80;   // display-timer payload, first vtable write
extern void* NativeVtbl1459a9ed8;   // display-timer payload, final vtable write
