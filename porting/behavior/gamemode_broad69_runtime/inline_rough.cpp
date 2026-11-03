// Uncounted control-flow drafts; intentionally non-compilable pending the
// original types, field identities, ABI and unresolved effects. Offsets and
// call addresses are evidence identifiers, not established source names.
namespace rough {

// exec 0x1428f03e0; decomp_improved/AFortGameModeAthena/HandleAllPlaylistLevelsVisible.c
void HandleAllPlaylistLevelsVisible(Object* self) {
    auto world = vcall_150(self);
    if (!world) return;
    auto state = field_ptr(world, 0x130);
    if (!state || !is_a(state, AFortGameStateAthena_StaticClass())) return;
    FUN_140690620(field_address(state, 0x610), self);
    auto x = FUN_141999ba0(state);
    if (x && FUN_141c82200(x) && !FUN_14199e3c0(state) && !FUN_141c5fb10(x))
        FUN_141c70f80(x);
}

// exec 0x1428f0650; decomp_improved/AFortGameModeAthena/OnEndGameKickPlayers.c
void OnEndGameKickPlayers(Object* self) {
    if (field_ptr(self, 0x278)) AFortGameStateAthena_StaticClass();
    if (field_ptr(self, 0x3d8)) {
        auto world = vcall_150(self);
        auto iter = FUN_14468fb70(world); // weak-object array and index; no world null check
        while (iter.index >= 0 && iter.index < iter.count()) {
            auto player = FWeakObjectPtr_Get(iter.entry(iter.index));
            if (player && is_a(player, AFortPlayerControllerAthena_StaticClass())) {
                // Decompiler omitted arguments; recover their ABI before promotion.
                if (FUN_144481860_unresolved_args()) FUN_142a3bcb0_unresolved_args();
            }
            ++iter.index;
        }
        FUN_1419602e0(self);
    }
}

// exec 0x1428f0420; decomp_improved/AFortGameModeAthena/JumpToSafeZonePhase.c
void JumpToSafeZonePhase(Object* self) {
    auto state = field_ptr(self, 0x278);
    if (!state || !is_a(state, AFortGameStateAthena_StaticClass())) return;
    auto phase = field_u8(state, 0x1cb0);
    if (static_cast<unsigned char>(phase - 2u) >= 3) return;
    if (phase == 2) {
        field_u32(self, 0xd78) = FUN_14193af00(self, 0);
        FUN_14196b640(self, 1);
    } else if (phase == 3) {
        for (int i = field_i32(state, 0x1ee8) - 1; i >= 0; --i) {
            auto entry = FUN_141990d90(state, i);
            if (entry) FUN_143f6c200(entry, 0, 1);
        }
    }
    FUN_141971930(self, 0, 1);
    auto world = vcall_150(self);
    if (!world) return;
    auto iter = FUN_14468fb70(world);
    while (iter.index >= 0 && iter.index < iter.count()) {
        auto player = FWeakObjectPtr_Get(iter.entry(iter.index));
        if (player && is_a(player, AFortPlayerControllerAthena_StaticClass()) &&
            !player_field_NamePrivate_0xe(player)) {
            auto x = player_field_NamePrivate_0xd(player);
            if (!(x && is_a(x, FUN_142a8ae70()) && field_u8(x, 0x12ec))) {
                if (FUN_14470a4e0(self, player)) {
                    vcall_778(self, player);
                    auto nested = FUN_1419b90a0(/* stack iterator */, 0, self, 0);
                    while (nested.current && vcall_728(nested.current, player) != 2)
                        FUN_1419f3d90(nested, nested.index_2c + 1);
                    FUN_1419bb7e0_unresolved_args();
                }
            }
        }
        ++iter.index;
    }
}
} // namespace rough
