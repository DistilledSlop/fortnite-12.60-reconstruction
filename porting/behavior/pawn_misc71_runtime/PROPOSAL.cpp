// Uncounted rough lifts. All types, byte offsets, virtual calls and FUN adapters
// are unresolved; this is reviewable control flow, not compilable UE code.
namespace pawn_misc71 {

// 142a80fe0: the archive adapter exposes the exact direct-buffer versus virtual
// serialization paths; its state includes the original cursor/end and flag word.
void UpdateAimbotDetectionStatus(Pawn& p) {
    auto* actor = p.ptr_at_250();
    if (!actor || !actor->vcall_6a8()) return;
    auto& bytes = p.storage_at_LargeBodyTypeTargetingOffset_34();
    Archive a{}; FUN_142ccbd80(a); a.set_vtable_DAT_1459ac360();
    a.set_storage(&bytes); FUN_142cd24e0(a, 1); a.vcall_d8(0);
    uint32_t marker = 0xfaaa;
    if (a.end() < a.cursor_as_u32() + 1) {
        if (a.vcall_150_is_FUN_1406a1750()) FUN_1406a1750();
        else a.vcall_150(&marker, 4);
        if (a.flags() & 0x2000) FUN_142ccd220(a, &marker, 4);
    } else { marker = a.read_u32_and_advance(); }
    Opaque24 value; p.vcall_6d8(value); FUN_1415408d0(a, value);
    uint8_t delta = p.VocalChords[0x18] - p.VocalChords[0x44];
    if (a.end() < a.cursor_as_byte() + 1) {
        if (a.vcall_150_is_FUN_1406a1750()) FUN_1406a1750();
        else a.vcall_150(&delta, 1);
    } else { delta = a.read_byte_and_advance(); }
    auto* weapon = p.CurrentWeapon; OpaquePtr weapon_value = nullptr;
    if (weapon && IsAByClassChain(weapon, FUN_142b4dac0()))
        weapon_value = weapon->vcall_6b0();
    actor = p.ptr_at_250(); uint32_t result = 0;
    if (actor && IsAByClassChain(actor, FUN_142a3d9f0())) {
        auto h = FUN_14447ee40(actor); auto x = FUN_14081b480(h);
        if (x) {
            auto c = FUN_14081b540(x);
            if (c == 1) { if (weapon_value) result = FUN_14266d180(weapon_value, 1); }
            else { if (weapon_value) result = FUN_142672660(weapon_value, 1); }
        }
    }
    if (a.end() < a.cursor_as_u32() + 1) {
        if (a.vcall_150_is_FUN_1406a1750()) FUN_1406a1750();
        else a.vcall_150(&result, 4);
        if (a.flags() & 0x2000) FUN_142ccd220(a, &result, 4);
    } else { result = a.read_u32_and_advance(); }
    int count = p.int_at_LargeBodyTypeTargetingOffset_3c();
    ++bytes.data()[count - 1];
    int random = rand();
    if (float(random) * 3.051851e-05f < 0.5f && count - 1 < 100)
        FUN_142a774e0(p, bytes);
    FUN_142ccc820(a);
}

// 142a7e240
void OnRep_WaitingForEmoteInteraction(Pawn& p) {
    auto* component = p.EmoteInteractCollisionComponent;
    if (!(p.HeldObject[0x1c] & 2)) {
        if (component) {
            component->vcall_3a0(0);
            auto fn = p.EmoteInteractCollisionComponent->vfunc_138();
            auto name = FUN_142e80820();
            fn(p.EmoteInteractCollisionComponent, 0, name, 0x15);
            p.EmoteInteractCollisionComponent = nullptr;
        }
    } else if (!component) {
        NameTemp name; auto* named = FUN_142cd4ab0(name, L"EmoteInteractCollision", 1);
        auto created = FUN_14197e8d0(p, *named, 0, 0, 0, 0);
        auto parent = p.ptr_at_130(); p.EmoteInteractCollisionComponent = created;
        name.zero_raw();
        FUN_144125240(created, parent, 0);
        FUN_1440fc970(p.EmoteInteractCollisionComponent, 0);
        p.EmoteInteractCollisionComponent->vcall_620(p.EmoteInteractionCollisionProfile, 1);
        Raw12 extent = p.EmoteInteractionBoxExtent;
        FUN_1440c9760(p.EmoteInteractCollisionComponent, extent, 0);
        FUN_1440c2bb0(p.EmoteInteractCollisionComponent);
    }
}

// 142a7dd90: both OnRep_GroupEmoteLookTarget and OnRep_IsEmoteLeader
// dispatch here; one body. Raw float bit copies and returned packed value retained.
void OnRep_GroupEmoteLookTarget_IsEmoteLeader(Pawn& p) {
    if ((p.HeldObject[0x1c] & 4) && p.GroupEmoteLookTarget) {
        auto* targetRoot = p.GroupEmoteLookTarget->ptr_at_130();
        RawVec3 target = targetRoot ? targetRoot->raw_vec_at_1d0() : DAT_148218630_638();
        auto* ownRoot = p.ptr_at_130();
        RawVec3 own = ownRoot ? ownRoot->raw_vec_at_1d0() : DAT_148218630_638();
        RawVec3 difference = subtract_components(target, own);
        auto* rotation = FUN_142c5cdf0(difference, own);
        p.write_u64_GroupEmoteFollowers_1c(rotation->first_u64());
        p.write_u32_GroupEmoteFollowers_1c(0);
        p.write_u32_GroupEmoteFollowers_24(0);
    }
}

// 142a7e100: preserve saved ASC for copy, but reread field for virtual call.
void OnRep_ReplicatedAnimMontage(Pawn& p) {
    auto* asc = p.AbilitySystemComponent;
    if (asc && IsAByClassChain(asc, FUN_14277e480())) {
        FUN_1406daaf0(asc, p.RepAnimMontageInfo);
        copy_exact_order_8_8_4_4_4_4_8(asc->at_14a8(), p.LandingMontagePair_tail());
        p.AbilitySystemComponent->vcall_7d8();
    }
}

// 14234a150: apparent argument types/register high bits are unresolved.
void ServerChoosePart_Implementation(Pawn& p, RawArg chosen, RawArg part) {
    auto* owner = FUN_141ee5f70();
    auto* component = owner ? owner->ptr_at_5d8() : (p.Hero ? p.HACK_CustomPRIComponent : nullptr);
    if (!component) return;
    FUN_141db8ac0(component, low32(chosen), part); // high seven bytes of part unknown
    bool changed = false; uint8_t i = 0;
    do {
        if (uint32_t(i) < 6) {
            auto* parts = p.vcall_1250(0);
            if (parts && parts[i] != p.PreviousCharacterParts[i] && p.CharacterPartSkeletalMeshComponents[i]) {
                FUN_142346450(p, uint64_t(i)); p.vcall_11f0(); changed = true;
            }
        }
        i = uint8_t(i + 1);
    } while (i < 6);
    if (changed) {
        FUN_14232dfc0(p); FUN_142a767c0(p);
        owner = FUN_141ee5f70(p);
        component = owner ? owner->ptr_at_5d8() : (p.Hero ? p.HACK_CustomPRIComponent : nullptr);
        if (component) FUN_141db5c70(); // decompiler did not recover arguments
    }
}

// 142a1e550
void OnAllMontageInstancesEnded(Pawn& p) {
    int mode = FUN_143f7a920();
    if (mode == 1 && (p.UroShiftBucket[1] & 1) && p.ptr_at_278()) {
        uint8_t b = p.vcall_d88();
        p.ptr_at_278()->byte_at_574() = b;
        p.ptr_at_278()->vcall_340(b < 3);
    }
    if (p.byte_at_f0() == 2) {
        p.ptr_at_280()->vcall_ad0();
        p.byte_at_521() &= 0xfe;
    }
}
}
