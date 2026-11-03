// Uncounted C++-shaped control-flow lifts. Helpers, layouts, arguments and
// ownership below are unresolved evidence identifiers, not buildable APIs.
namespace rough69 {
// 0x1419331a0; AddAIClassToReplicationGraph native target.
void AddAI(Word self, Word cls) {
    ++global_1478c2684;
    bool dirty = false;
    for (int64 i = global_1478c2678 - 1; i >= 0; --i) {
        auto& e = global_1478c2670[i]; // 16-byte entries
        if (!e.word8 || !e.object || !vcall_50(e.object, cls)) dirty = true;
    }
    int oldCount = global_1478c2678;
    --global_1478c2684;
    if (dirty && global_1478c2684 < 1) {
        int i = 0;
        while (i < global_1478c2678) {
            auto& e = global_1478c2670[i];
            if (!e.word8 || !e.object || vcall_20_unresolved_args(e.object))
                FUN_140690780(&global_1478c2670, i, 1, 1);
            else ++i;
        }
        global_1478c2680 = global_1478c2678 * 2;
        if (global_1478c2680 < 3) global_1478c2680 = 2;
        if (global_1478c2680 < oldCount && global_1478c267c != global_1478c2678)
            FUN_1406911b0(&global_1478c2670);
    }
}
// 0x14193dc90 and 0x141946800; unreachable-block warnings in decompiler.
// Original call arguments and 0x20-byte callable's uninitialized slot unresolved.
void EnableGameplayMutator(Word self, Word arg) {
    OpaqueContainer local{nullptr, 0, 0xffffffff};
    Word captured = arg;
    FUN_1419d5690(&local, 0);
    auto p = FMemory_Realloc(nullptr, 0x20, 0);
    if (p) { slot(p,0) = &DAT_145e063f0; slot(p,1) = &captured; slot(p,3) = FUN_142bad740_unresolved_args(); }
    FUN_1419d57d0_unresolved_args(&local);
    if (p) {
        vcall_38_unresolved_args(p);
        auto q = FMemory_Realloc(p, 0, 0);
        if (q) FMemory_Free(q);
    }
    if (local.data) FMemory_Free(local.data);
}
Word GetGameplayMutator(Word self, Word arg) {
    OpaqueContainer local{nullptr, 0, 0xffffffff};
    Word captured = arg;
    FUN_1419d5690(&local, 0);
    auto p = FMemory_Realloc(nullptr, 0x20, 0);
    if (p) { slot(p,0) = &DAT_145e063f0; slot(p,1) = &captured; slot(p,3) = FUN_142bad740_unresolved_args(); }
    Word result = FUN_1419d5560_unresolved_args(&local);
    if (p) {
        vcall_38_unresolved_args(p);
        auto q = FMemory_Realloc(p, 0, 0);
        if (q) FMemory_Free(q);
    }
    if (local.data) FMemory_Free(local.data);
    return result;
}
// 0x14195e060; static RemoveAllLootFromWorld target. Parameters are raw ABI.
void RemoveLoot(Word context, Word32 a, Word32 b, Word32 c, Byte d) {
    if (context) {
        auto world = FUN_144640410(global_148351eb8, context, 1);
        if (world) {
            auto mode = ptr(world, 0x128);
            if (mode && is_a(mode, FUN_1428ee970())) FUN_14195e100(mode,a,b,c,d);
        }
    }
}
// 0x141972f90; static StopRemovingAllLoot target.
void StopLoot(Word context, Byte immediate) {
    auto world = FUN_144640410(global_148351eb8, context, 1);
    if (!world) return;
    auto mode = ptr(world, 0x128);
    if (!mode || !is_a(mode, FUN_1428ee970())) return;
    auto handle = &word64(mode, 0x1570);
    auto snapshot = *handle;
    auto manager = FUN_140f95ae0(mode);
    auto timer = FUN_144621fb0(manager, &snapshot);
    if (timer && byte(timer, 1) != 2) {
        if (immediate) FUN_14195e100(mode, 0xffffffff, 0xffffffff, 0xbf800000, 1);
        manager = FUN_140f95ae0(mode);
        if (FUN_144621fb0(manager, handle)) FTimerManager_InternalClearTimer(manager, *handle);
        *handle = 0;
    }
}
// 0x141949f60; GetVehicleClassOverride. resolve_soft_entry below explicitly
// represents the weak/soft cache branch, including its cache side effects.
Word* VehicleOverride(Word self, Word* out, Word requested) {
    auto begin = ptr(self,0x1330);
    auto end = begin + int32(self,0x1338)*0x60;
    for (auto e = begin; e != end; e += 0x60) {
        bool inspect = int32(e,8) < 2;
        if (!inspect) {
            auto x = FUN_143f76130(self);
            if (!x) inspect = true;
            else if (!is_a(x,FUN_1428ea7f0())) inspect = true;
            else {
                auto y = FUN_141bd9690(x);
                if (!y || FUN_142261550(y,e)) inspect = true;
            }
        }
        if (!inspect) continue;
        auto candidate = FWeakObjectPtr_Get(e+0x10);
        if (!candidate && (int32(e,0x20) || int32(e,0x24))) {
            atomic_compare_exchange_zero_noop(global_147c4e988);
            if (global_147c4e988 != int32(e,0x18) || int32(e,0x10) != -1) {
                auto resolved = FUN_142ed4d40(e+0x20);
                FUN_142ee3b30(e+0x10,resolved);
                if (resolved || !global_148262b59) {
                    atomic_compare_exchange_zero_noop(global_147c4e988);
                    int32(e,0x18) = global_147c4e988;
                }
                candidate = FWeakObjectPtr_Get(e+0x10);
            }
        }
        if (candidate && (!is_a(candidate,FUN_142d68560()) ||
            !class_is_child_of(candidate,FUN_1446acb00()))) candidate = nullptr;
        auto validRequested = requested && class_is_child_of(requested,FUN_1446acb00()) ? requested : nullptr;
        if (candidate == validRequested) {
            requested = FUN_1407d0ea0(e+0x38);
            if (requested && (!is_a(requested,FUN_142d68560()) ||
                !class_is_child_of(requested,FUN_1446acb00()))) requested = nullptr;
            break;
        }
    }
    *out = requested;
    return out;
}
// 0x141953840; OnGamePhaseStepChanged native. The pointer and byte argument
// meanings contradict exec labels; retain ABI, not those unreliable labels.
void PhaseStep(Word self, Word* arg2, Byte arg3) {
    auto state = *arg2;
    if (!state || !is_a(state,AFortGameStateAthena_StaticClass())) return;
    for (unsigned i = 0; int(i) < int32(self,0x1380); ++i) {
        auto entry = ptr(self,0x1378) + i*0x20;
        auto x = ptr(entry,0);
        if (x && byte(x,0x48) == 1 && arg3 == 7) {
            auto gameState = ptr(self,0x278);
            if (gameState && is_a(gameState,AFortGameStateAthena_StaticClass()) &&
                float(byte(gameState,0x1eb9)) == FScalableFloat_GetValueAtLevel(x+0x50,0.0f,nullptr))
                FUN_141967e60(self,entry);
        }
    }
    if (Byte(arg3-7u) < 2) {
        word32(self,0xa38) = vcall_630(state);
        if (arg3 == 7) {
            auto begin = ptr(self,0xdd0);
            auto end = begin + int32(self,0xdd8)*sizeof(Word);
            for (auto it = begin; it != end; it += sizeof(Word)) {
                auto p = ptr(it,0);
                if (!p) continue;
                auto x = vcall_1bd0(p,2);
                if (!x) continue;
                OpaqueArray a{}, b{}, c{}, d{}, e{}, f{};
                FUN_142442c50(x,&a,&c); // exact aggregate layout unresolved
                vcall_14d8_unresolved_args(p);
                U_OriginalEmptyTarget(); // emitted Stub_Empty, address unresolved; no invented effect
                if (d.data) FMemory_Free(d.data);
                if (c.data) FMemory_Free(c.data);
                if (b.data) FMemory_Free(b.data);
                if (a.data) FMemory_Free(a.data);
                if (f.data) FMemory_Free(f.data);
                if (e.data) FMemory_Free(e.data);
            }
        }
    }
}
// 0x141952900; OnCheckForOrphanedServer native. Delegate layout, timer
// setup call arguments, and virtual cleanup receiver selection remain opaque.
void CheckOrphan(Word self, Byte fromLogout) {
    auto world = vcall_150_unresolved_args(self);
    if (!world || !word64(self,0x3d8) || vcall_6a0(self) != 0 || !vcall_6b8(self)) return;
    auto timerManager = [&]() {
        auto x = ptr(world,0x188);
        return x ? ptr(x,0xc0) : ptr(world,800);
    };
    for (auto offset : {0xb90,0xdc0,0xba0}) {
        auto manager = timerManager();
        // Third lookup's handle argument was omitted by the decompiler.
        auto found = offset == 0xba0 ? FUN_144621fb0_unresolved_args(manager)
                                    : FUN_144621fb0(manager,&word64(self,offset));
        if (found) FTimerManager_InternalClearTimer(manager,word64(self,offset));
        word64(self,offset) = 0;
    }
    if (!vcall_818_unresolved_args(self)) vcall_828_unresolved_args(self);
    if (global_1481348e0 > 4)
        FMsg_Logf_Internal(nullptr,0,&DAT_1481348e4,5,fromLogout ?
            L"FortGameModeAthena: Logout called on final player.  Restarting session." :
            L"FortGameModeAthena: Server detected to have been orphaned.  Restarting session.");
    OpaqueWeak weak;
    FUN_142ee3b30(&weak,self);
    OpaqueDelegate original = make_opaque_delegate(&DAT_1459feee8,weak,FUN_141936b50);
    auto manager = timerManager();
    OpaqueHolder holder{};
    OpaqueWeak nullWeak;
    FUN_142ee3b30(&nullWeak,0);
    OpaqueDelegate copy = transfer_delegate_fields_and_clear_callback(original, nullWeak);
    FUN_144627d90(manager, /* out stack e0 */, &holder, /* copy: ABI unresolved */);
    if (copy.callback) vcall_10_unresolved_args(copy.inline_or_heap());
    auto selected = holder.count ? holder.pointer : nullptr;
    if (selected || (holder.count && holder.pointer)) {
        if (!selected) selected = holder.pointer;
        vcall_38(selected);
        if (holder.pointer) holder.pointer = FMemory_Realloc(holder.pointer,0,0);
        holder.count = 0;
    }
    if (!holder.count && holder.pointer) FMemory_Free(holder.pointer);
    if (original.callback) vcall_10_unresolved_args(original.inline_or_heap());
    stack_cookie_check();
}
} // namespace rough69
