// Behaviour snippet: AFortCarriedObject::GetHoldingActor (body only; spliced into the generated member function).
// Original address: 0x14094dff0 (asm: MOV RAX,[RCX+0x28]; RET). Exec thunk 0x14285c6c0.
// Exec thunk: ADD RCX,0x218; CALL [RAX+0x10]. Secondary vptr at +0x218 = 0x145f73548 (vtables.json), slot +0x10 = 0x14094dff0 (runtime image).
// Adjusted read: obj+0x218+0x28 = obj+0x240 = HolderInfo (FCarriedObjectAttachmentInfo, offset 0) = AttachParent (AActor*).
// Declared in generated header: no; reflection declares it (return class VA 0x1446ab9e0 = AActor).
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this) + 0x218;   // adjusted this (secondary base at +0x218)
return *reinterpret_cast<AActor* const*>(Raw + 0x28);
