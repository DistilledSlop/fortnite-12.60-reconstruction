// Uncounted control-flow draft, non-buildable. Source: exec 0x1428f08d0,
// porting/generated/decomp_improved/AFortGameModeAthena/SafeZoneInsideChecks.c.
// Names U_* represent unresolved behavior/ABI, not implementations or defaults.
namespace rough69 {
// Shared decompiler iterator advance (two duplicated source loops). Its array
// mutations and filtering must preserve evaluation order and repeated reads.
bool AdvanceSafeZoneIterator(OpaqueIterator& it) {
    auto world = it.world;
    for (int i = it.currentIndex + 1; i < it.firstCount + it.secondCount; ++i) {
        auto object = i < it.firstCount ? it.first[i] : it.second[i-it.firstCount];
        ++it.examinedCount;
        if (!object) continue;
        auto x = FUN_143f76730(object);
        if (!x) continue;
        if ((it.flags & 1) && U_GlobalObjectFlagBit29(object)) continue;
        // Original decompiler names this Stub_ReturnFalse; preserve the call,
        // not an assumed false value, pending identification of actual target.
        if ((it.flags & 2) && !U_OriginalReturnFalseTarget()) continue;
        if (it.flags & 4) {
            bool flags = (byte(x,500)&0x20) || (byte(x,0x1f5)&0x40);
            auto owner = ptr(x,0x248);
            bool same = !ptr(x,0xb8);
            if (!same) {
                auto current = FUN_14468e190_unresolved_args();
                same = !current || owner == current;
            }
            bool allowedOwner = !owner || byte(owner,0) == 2;
            if (!flags || !(same || allowedOwner)) continue;
        }
        if (FUN_1413064f0(x) == world &&
            (x == ptr(world,0x30) || !is_a(object,FUN_14480eb00()))) {
            it.currentIndex = i;
            it.current = object;
            return !it.done;
        }
    }
    it.current = nullptr;
    it.done = true;
    return false;
}
void SafeZoneInsideChecks(Word self) {
    U_StackCookie cookie;
    auto stat = global_1482472e4 ? nullptr : L"STAT_SafeZoneInsideCheck";
    if (!byte(self,0xda0)) {
        auto state = ptr(self,0x278);
        if (state && !is_a(state,AFortGameStateAthena_StaticClass())) state = nullptr;
        // The decompiler dereferences state even when null; no guard invented.
        if (byte(state,0x1cb0) == 2) {
            auto x = FUN_141991bf0(state);
            Byte enabled = x && byte(x,0x8c1) != 0;
            auto klass = FUN_142a82010();
            auto world = vcall_150(self);
            OpaqueIterator it = FUN_14192cca0(world,klass,5);
            if (!it.done) do {
                auto obj = it.current;
                Byte second = enabled ^ 1;
                Byte first = enabled;
                if (FUN_14232d430(obj)) first ^= 1;
                FUN_14234d960(obj,first);
                vcall_1010(obj,second);
            } while (AdvanceSafeZoneIterator(it));
            if (it.cleanupFlag) {
                it.cleanupFlag = false;
                FUN_14469a960(it.world,it.cleanupToken);
                if (it.second) FMemory_Free(it.second);
                if (it.first) FMemory_Free(it.first);
            }
        } else {
            auto arg = FUN_1419494f0(self);
            Byte flag = thunk_FUN_14199e4e0(state);
            auto optional = FUN_14199e430(state) ? FUN_141c74820(self) : nullptr;
            auto klass = FUN_142a82010();
            auto world = vcall_150(self);
            OpaqueIterator it = FUN_14192cca0(world,klass,5);
            if (!it.done) do {
                auto obj = it.current;
                auto component = ptr(obj,0x130);
                auto position = component ? U_ReadVector(component+0x1d0) : U_GlobalVector_148218630();
                Byte inside = FUN_14194e9e0(self,&position,arg);
                int index = int32(self,0xfe0);
                if (optional && inside) {
                    auto group = FUN_141c7c5c0(optional,obj);
                    if (group && int32(group,0x4f8) > 0) {
                        bool withinAny = false;
                        int maxIndex = -1;
                        for (int i=0; i<int32(group,0x4f8); ++i) {
                            auto volume = ptr(ptr(group,0x4f0),i*sizeof(Word));
                            if (!volume || U_GlobalObjectFlagBit29(volume)) continue;
                            auto shape = volume+0x218;
                            if (Byte(vcall_30_unresolved_args(shape)-7u) >= 2) continue;
                            if (!withinAny) {
                                float radius = vcall_40_unresolved_args(shape);
                                auto center = vcall_48(shape);
                                float dx = position.x-center.x, dy = position.y-center.y;
                                if (dx*dx+dy*dy <= radius*radius) withinAny = true;
                            }
                            if (maxIndex < int32(volume,0x45c)) maxIndex = int32(volume,0x45c);
                        }
                        if (maxIndex != -1) {
                            if (!withinAny) { inside = 0; index = maxIndex; }
                        } else index = 0;
                    }
                }
                // Two byte stack arguments precede call; exact prototype unresolved.
                FUN_1419622e0_unresolved_abi(self,state,obj,index,inside,flag);
            } while (AdvanceSafeZoneIterator(it));
            if (it.cleanupFlag) {
                it.cleanupFlag = false;
                FUN_14469a960(it.world,it.cleanupToken);
                if (it.second) FMemory_Free(it.second);
                if (it.first) FMemory_Free(it.first);
            }
        }
    }
    if (global_1482472e4 && stat) FUN_142cd3770(&stat);
    U_CheckStackCookie(cookie);
}
} // namespace rough69
