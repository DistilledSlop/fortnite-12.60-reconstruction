// Legacy held-input native interface and manager tick state.
// Source: original image-125.runtime.bin, VA base 0x140000000.
//
// Slot index = vtable offset / 8. The interface has exactly seven slots, in this
// order, shared by both concrete implementations:
//   +00 deleting destructor thunk        global 1407a5da8   panel 1407a5d84
//   +08 owner-object getter              global 140695430   panel 1408056ac
//   +10 input dispatch A (BLOCKED)       global 1407c8be0   panel 1407c87e0
//   +18 input dispatch B (BLOCKED)       global 1407cab10   panel 1407ca7f0
//   +20 input dispatch C (BLOCKED)       global 1407cc2b0   panel 1407cbf10
//   +28 held tick (float in XMM1)        global 140802440   panel 140802200
//   +30 unrecovered slot 6 (BLOCKED)     global 140800b70   panel 140800860
// Vtables: global 1459f1338 (object+0x28), panel 1459ee038 (object+0x2d8).
// Names are descriptive; original interface source names were not recovered.

// Held pair stored on UCommonInputManager. Words are at manager+0xa8 (object)
// and manager+0xb0 (interface). The interface word is the secondary-base
// sub-object of the owning handler, not the owner itself.
struct FLegacyHeldInputHandlerPair
{
    UObject* Object = nullptr;                       // manager+0xa8
    class ILegacyHeldInputNative* Interface = nullptr; // manager+0xb0

    UObject* GetObject() const { return Object; }
    bool IsValid() const { return Object != nullptr && Interface != nullptr; }
};

// Native interface (secondary base). Pure virtuals match the seven vtable slots.
class ILegacyHeldInputNative
{
public:
    virtual ~ILegacyHeldInputNative() = default;      // +00

    // +08: returns the owning UObject (global: this-0x28; panel: this-0x2d8).
    virtual UObject* NativeGetOwnerObject() const = 0;

    // +10, +18, +20: input dispatch slots. Parameter lists NOT recovered (see NOTES.md).
    virtual void NativeBlockedDispatchSlot2() = 0;    // SIGNATURE BLOCKED
    virtual void NativeBlockedDispatchSlot3() = 0;    // SIGNATURE BLOCKED
    virtual void NativeBlockedDispatchSlot4() = 0;    // SIGNATURE BLOCKED

    // +28: advances held duration for the current owner. XMM1 carries DeltaSeconds.
    virtual void NativeHeldTick(float DeltaSeconds) = 0;

    // +30: unrecovered slot. Signature and semantics BLOCKED (see NOTES.md).
    virtual void NativeBlockedSlot6() = 0;            // SIGNATURE BLOCKED
};

// Member block for UCommonInputManager (tickable secondary base at manager+0x28).
// Offsets are absolute manager offsets as used by the original tick 1407ffc50.
public:
    virtual void Tick(float DeltaSeconds) override;   // original 1407ffc50 via secondary table 1459ed5f8
    // Returns true when the operation-suspension branch ran (suspended), false otherwise.
    bool NativeTickOperationSuspension(float DeltaSeconds);
private:
    bool bNativeOperationSuspended = false;            // +fd
    bool bNativeOperationProcessing = false;           // +fc
    bool bNativeExternalSuspension = false;            // +fe, not gated by tick
    int32 NativeIdleTickCount = 0;                     // +100
    // Operations TArray: header at +e8, count at +f0 (element type descriptive).
    TArray<FCommonInputOperationRecordNative> NativeOperations; // +e8
    FCommonInputSuspensionChangedNative NativeInputSuspensionChanged; // +90 (native bool multicast)
    FLegacyHeldInputHandlerPair CurrentlyHeldActionInputHandler;      // +a8 / +b0

// Member block for UCommonGlobalInputHandler: ILegacyHeldInputNative secondary base at +0x28.
public:
    virtual UObject* NativeGetOwnerObject() const override;
    virtual void NativeBlockedDispatchSlot2() override;   // BLOCKED
    virtual void NativeBlockedDispatchSlot3() override;   // BLOCKED
    virtual void NativeBlockedDispatchSlot4() override;   // BLOCKED
    virtual void NativeHeldTick(float DeltaSeconds) override; // BLOCKED: 140802440
    virtual void NativeBlockedSlot6() override;           // BLOCKED

// Member block for UCommonActivatablePanel: ILegacyHeldInputNative secondary base at +0x2d8.
public:
    virtual UObject* NativeGetOwnerObject() const override;
    virtual void NativeBlockedDispatchSlot2() override;   // BLOCKED
    virtual void NativeBlockedDispatchSlot3() override;   // BLOCKED
    virtual void NativeBlockedDispatchSlot4() override;   // BLOCKED
    virtual void NativeHeldTick(float DeltaSeconds) override; // BLOCKED: 140802200
    virtual void NativeBlockedSlot6() override;           // BLOCKED
