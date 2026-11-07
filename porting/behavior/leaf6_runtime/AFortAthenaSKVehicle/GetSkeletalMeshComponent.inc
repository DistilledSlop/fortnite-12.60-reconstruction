// Behaviour snippet: AFortAthenaSKVehicle::GetSkeletalMeshComponent (body only; spliced into the generated member function).
// Original address: 0x1418ee570 (asm: MOV RAX,[RCX+0xaf8]; RET). Thunk 0x142833e70 is the exec stub.
// Exec thunk: ADD RCX,0x3b0; MOV RAX,[RCX]; CALL [RAX+0x598]. Secondary vptr at +0x3b0 = 0x145dd6610 (vtables.json),
// slot +0x598 of that table = 0x1418ee570 (read from FortniteClient-Win64-Shipping.runtime.exe). Body receives this+0x3b0.
// Adjusted read: obj+0x3b0+0xaf8 = obj+0xea8 = AFortAthenaSKVehicle::SkeletalMesh (reflection offset 0xea8, USkeletalMeshComponentBudgeted*).
// Declared in generated header: no; reflection declares it (implements UFortVehicleInterface::GetSkeletalMeshComponent).
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this) + 0x3b0;   // adjusted this (secondary base at +0x3b0)
return *reinterpret_cast<USkeletalMeshComponent* const*>(Raw + 0xaf8);
