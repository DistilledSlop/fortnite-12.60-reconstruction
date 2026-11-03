// panel_pop_stack_runtime: declarations recovered from image-125 (0x1407e1130, 0x1407e0c80).
// Fragment only. Not registered, not compiled. See NOTES.md.

// UCommonInputManager (reflected: ActivatablePanelStack +0xb8 offset 184, Operations +0xe8 offset 232)
//   void PopActivatablePanel(UCommonActivatablePanel* ActivatablePanel);   // 0x1407e0c80
//   TArray<UCommonActivatablePanel*> NativePredictedPanelStack() const;    // 0x1407ad3f0 (legacy name)
//   void NativeBeginOperationProcessing();                                 // 0x1407ab920 (legacy name)

// UCommonActivatablePanel : UCommonActivatableWidget
//   TWeakObjectPtr<UCommonInputManager> NativeInputManager;   // +0x3b0 (usage-verified, not reflected)
//   bool bNativeInActivationStack = false;                    // +0x304 (legacy panel-declarations.inc)
//   virtual void NativeAddedToActivationStack();              // vtable +0x4e8
//   virtual void NativeRemovedFromActivationStack();          // vtable +0x4f0
//   virtual void <unresolved slot +0x4c0>();                  // 0x1407e1130: pops self via NativeInputManager

// UCommonActivatableWidget
//   bool bIsActive;                                           // +0x288 (bitfield, byte offset from HANDOFF only)

// FOperation (0x28 bytes): EOperation Operation @0 (Intro=0, Outro=1, Push=2, Pop=3), UCommonActivatablePanel* Panel @8
