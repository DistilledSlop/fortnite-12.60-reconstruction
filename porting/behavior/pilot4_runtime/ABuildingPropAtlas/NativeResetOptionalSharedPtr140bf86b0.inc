// Helper 0x140bf86b0 (28 insns): RCX = &local optional (caller 0x141a3d950 passes LEA RCX,[RBP+0x140] before the
// frame teardown; it is the scope exit of an optional shared pointer).
// Asm: if (byte [RCX+0x18]) { byte [RCX+0x18] = 0; Controller = [RCX+8]; if (Controller) release shared reference }.
// Release = LOCK XADD -1 on [Controller+8]; when it was 1 call vtable[0]; LOCK XADD -1 on [Controller+0xc]; when it was 1
// call vtable[1]. That is TSharedPtr<T, ThreadSafe>::Reset (object pointer at +0, controller at +8; type erased).
// CONFIDENCE: MEDIUM (the object pointer field is nulled by Reset, the asm leaves it untouched)
if (OptionalStorage[0x18] != 0)
{
    OptionalStorage[0x18] = 0;
    reinterpret_cast<TSharedPtr<uint8, ESPMode::ThreadSafe>*>(OptionalStorage)->Reset();
}
