// Original body address: 0x143fccf40 (helper, no thunk)
// Offset evidence: asm is MOV RAX,qword ptr [RCX + 0x20]; MOV RAX,qword ptr [RAX + 0xa0]; RET. +0x20 is UObject::OuterPrivate, read here through GetOuter(). No reflected member covers +0xa0 on the outer object.
// Asm dereferences the outer object without a null check, and the body does the same.
// Callers: UCustomCharacterPartAnimInstance::GetAssociatedPlayerPawn (0x141d7bc20) and CharacterItemDefinitionHasAnyMetaTag (0x141d72be0). Both test the returned object's class before use.
// CONFIDENCE: HIGH
UObject* OuterObject = AnimInstance->GetOuter();
return *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(OuterObject) + 0xa0);
