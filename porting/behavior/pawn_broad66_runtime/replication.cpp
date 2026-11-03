// Rough lifts; Native_* calls retain effects whose implementation/signature is
// not fully recovered. Offsets refer to the original client layout.
void AFortPlayerPawn::OnRep_AnimBPOverride()
{
    auto* Mesh = GetMesh(); // pawn +0x278
    if (!Mesh) return;
    UClass* Override = Native_ValidAnimClass(AnimBPOverride); // 0x1446b91c0 cast
    if (!Override) {
        if (!Native_ValidAnimClass(OriginalAnimBP)) return;
        if (auto* Instance = Native_GetAnimInstance_144114bd0(Mesh))
            Native_AnimInstanceReset_143fe17f0(Instance, false);
        Native_SetAnimInstanceClassInternal_14234b2e0(this, OriginalAnimBP);
        OriginalAnimBP = nullptr;
    } else {
        if (!Native_ValidAnimClass(OriginalAnimBP))
            OriginalAnimBP = Mesh->AnimClass; // +0x628
        if (Native_ValidAnimClass(Mesh->AnimClass) == Native_ValidAnimClass(AnimBPOverride)) return;
        if (auto* Instance = Native_GetAnimInstance_144114bd0(Mesh))
            Native_AnimInstanceReset_143fe17f0(Instance, false);
        Native_SetAnimInstanceClassInternal_14234b2e0(this, AnimBPOverride);
    }
    Native_BroadcastDelegate_140684de0(this, 0x1c18);
    // Verbose log of role and selected class omitted; no gameplay mutation.
}

void AFortPlayerPawn::OnRep_AttachmentMesh()
{
    auto* Component = AttachmentMeshComponent;
    if (!AttachmentMesh.SkeletalMesh) {
        if (Component) {
            Native_DestroyComponent_1440d4a00(Component);
            Native_MarkObjectGarbage(Component); // object-array flag 0x20000000
            AttachmentMeshComponent = nullptr;
        }
        return;
    }
    if (!Component || Native_IsObjectGarbage(Component)) {
        Component = Native_NewBudgetedSkeletalMeshComponent_1406828e0(this);
        AttachmentMeshComponent = Component;
        Native_ConfigureAttachmentBits(Component); // +20a/b/e, +574, +38
        Native_SetCollisionProfile_v620(Component, FName("CharacterMesh"), true);
        Native_1440fc970(Component, false);
        Native_ClearBit_6f9_04(Component);
        Native_1440c9970(Component, false);
        Native_AttachToComponent_14410a300(Component, GetMesh(), Native_NoneName, Native_AttachmentRules);
        Native_1440ca810(Component, false);
    }
    if (!Native_ComponentRegistered(Component)) {
        Native_1440c2bb0(Component);
        Native_RegisterComponentWithOwner_v3b8(Component, this);
        Component = AttachmentMeshComponent;
    }
    Native_SetSkeletalMesh_v8a8(Component, AttachmentMesh.SkeletalMesh, true);
    if (Native_ValidAnimClass(AttachmentMesh.AnimBP))
        Native_SetAnimClass_144121b70(Component, Native_ValidAnimClass(AttachmentMesh.AnimBP));
}
