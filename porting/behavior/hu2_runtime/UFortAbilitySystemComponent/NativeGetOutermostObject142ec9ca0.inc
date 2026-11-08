// Helper 0x142ec9ca0 (static on UFortAbilitySystemComponent).
// Evidence: porting/generated/native-cap-direct/0x142ec9ca0.asm, 10 instructions, no calls.
// Behavior: follows the object pointer at +0x20 (UObject::OuterPrivate) until null and returns the last non-null object, or Context when it has no outer.
// Callers: 0x142e84670 (and others) use the returned pointer as an object; ABI RCX = object, returns RAX.
// CONFIDENCE: HIGH
UObject* CurrentObject = Context;
UObject* NextObject = CurrentObject->GetOuter();
while (NextObject != nullptr)
{
    CurrentObject = NextObject;
    NextObject = CurrentObject->GetOuter();
}
return CurrentObject;
