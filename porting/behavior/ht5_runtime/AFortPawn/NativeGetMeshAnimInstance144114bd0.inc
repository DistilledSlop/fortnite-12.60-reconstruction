// Helper body for AFortPawn static helper at 0x144114bd0 (asm: MOV RAX,qword ptr [RCX + 0x630]; RET). No branches, no calls.
// RCX is a USkeletalMeshComponent: callers pass [pawn+0x278] (ACharacter::Mesh) as RCX, e.g. AFortPawn::AnimMontage_LocalJumpToRandomSection 0x142304350,
// AFortPawn::PlayLocalAnimMontage 0x142341760, UFortAbilityTask_WaitAnimBPOverride::OnPawnAnimBPCallback 0x141620a10.
// Offset evidence: python3 -I porting/lookup_offset.py USkeletalMeshComponent 0x630 -> AnimScriptInstance (Object, +0x630, width 8).
// Read as a pointer and returned unchanged; callers test it for null.
// CONFIDENCE: HIGH
return *reinterpret_cast<UAnimInstance* const*>(reinterpret_cast<const uint8*>(MeshComp) + 0x630);
