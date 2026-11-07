// Behaviour snippet: AFortCharacterVehicle::GetSkeletalMeshComponent (body only; spliced into the generated member function).
// Original address: 0x141949740 (asm: MOV RAX,[RCX+(-0x2e20)]; RET). Exec thunk 0x1428618d0.
// Exec thunk: ADD RCX,0x3098; CALL [RAX+0x598]. Secondary vptr at +0x3098 = 0x145df83b0 (vtables.json), slot +0x598 = 0x141949740 (runtime image).
// Adjusted read: obj+0x3098-0x2e20 = obj+0x278 = ACharacter::Mesh (reflection offset 0x278, USkeletalMeshComponent).
// Declared in generated header: no; reflection declares it (implements UFortVehicleInterface::GetSkeletalMeshComponent).
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this) + 0x3098;   // adjusted this (secondary base at +0x3098)
return *reinterpret_cast<USkeletalMeshComponent* const*>(Raw - 0x2e20);
