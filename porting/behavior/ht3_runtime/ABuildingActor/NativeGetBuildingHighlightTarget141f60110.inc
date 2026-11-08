// Original body address: 0x141f60110 (helper, no thunk)
// Offset evidence: asm is MOV RAX,qword ptr [RCX + 0x258] then RET, so the 8-byte pointer at +0x258 is returned. No reflected member of ABuildingActor covers +0x258, so the raw load is used.
// Callers: ABuildingActor::SetQuestHighlight (0x141bc4dd0) and AFortPlayerController::SetBuildingHighlight (0x1423ae5d0) null-check the result and pass it on. The name follows those callers; the field's exact class is not reflected.
// CONFIDENCE: HIGH
return *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(BuildingActor) + 0x258);
