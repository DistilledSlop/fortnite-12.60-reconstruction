// Behavioral transcription of improved exec output, not buildable C++.
// U_* denotes unresolved calls, types, offsets, ABI, or ownership. Preserve raw
// values and evaluation order; reflected parameter comments may be misleading.

void StopRecordingHighlights(Controller* self, Frame* stack) { // exec 142a5d610
    P_FINISH(stack);
    if (byte_at(self->TalkingPlayersChanged, 0x10) == 0) return;
    auto world = self->vfn_150();
    if (world) {
        auto key = load8(self->RespawnCameraBehavior + 0xc);
        auto lookup = U_144621fb0(U_140f95ae0(self), &key);
        if (lookup && byte_at(lookup, 1) != 2) {
            RawCallback callback;
            callback.lo8 = address_1419fcc80;
            callback.word_8 = 0;
            // callback.word_c is not initialized in the recovered output.
            world = self->vfn_150();
            auto owner = ptr_at(world, 0x188);
            auto timer_manager = owner ? load8(owner, 0xc0) : load8(world, 800);
            auto delegate = FMemory_Realloc(nullptr, 0x30, 0);
            if (delegate) {
                store8(delegate, 0, &DAT_1459a9e80);
                U_142ee3b30(delegate + 8, self);
                store8(delegate, 0x10, callback.lo8);
                store4(delegate, 0x18, callback.word_8);
                store4(delegate, 0x1c, callback.word_c);
                store8(delegate, 0x28, U_142bad740());
                store8(delegate, 0, &DAT_1459a9ed8);
            }
            RawDelegateCopy copy = {nullptr, 0};
            if (delegate) U_virtual(delegate, 0x40, &copy);
            RawTimerArguments timer_args; // adjacent stack state, ABI unresolved
            U_142ee3b30(&timer_args.weak, nullptr);
            timer_args.word_a0 = 0xbf800000; timer_args.byte_a8 = 0;
            timer_args.word_70 = 0; timer_args.word_68 = 0; timer_args.ptr_58 = nullptr;
            U_1446278d0(timer_manager, self->RespawnCameraBehavior + 0x1c, &copy, 0x40b00000 /* 5.5f */);
            if (timer_args.word_68 != 0) {
                auto p = timer_args.ptr_58 ? timer_args.ptr_58 : &timer_args.inline_48;
                U_virtual(p, 0x10); // recovered call has no explicit arguments
            }
            auto p = copy.count ? copy.ptr : nullptr;
            if (p) {
                U_virtual(p, 0x38, p);
                if (copy.ptr) copy.ptr = FMemory_Realloc(copy.ptr, 0, 0);
                copy.count = 0;
                if (copy.ptr) FMemory_Free(copy.ptr);
            } else if (copy.count == 0) {
                if (copy.ptr) FMemory_Free(copy.ptr);
            } else if (copy.ptr) {
                // Same cleanup branch as p != nullptr, retained as in decompile.
                U_virtual(copy.ptr, 0x38, copy.ptr);
                copy.ptr = FMemory_Realloc(copy.ptr, 0, 0);
                copy.count = 0;
                if (copy.ptr) FMemory_Free(copy.ptr);
            }
            if (delegate) {
                U_virtual(delegate, 0x38, delegate);
                auto remaining = FMemory_Realloc(delegate, 0, 0);
                if (remaining) FMemory_Free(remaining);
            }
            return;
        }
    }
    U_threadsafe_static_init_if_needed(&DAT_148114a18, [&] {
        U_142cd49c0(&DAT_148114a10, "Highlight", 1);
    });
    auto name = DAT_148114a10;
    auto registry = U_142bad780();
    auto count = U_virtual(registry, 8, registry, name);
    name = DAT_148114a10;
    if (count > 0) {
        registry = U_142bad780();
        auto entry = U_virtual(registry, 0x10, registry, name, 0);
        auto object = entry ? entry - 8 : nullptr; // null is still dereferenced below in source
        U_virtual(object, 0x18, object, &self->HighlightGroupId);
        U_virtual(object, 0x30, object, &self->HighlightGroupId);
    }
}

void PrepareClientForRespawning(Controller* self, Frame* stack) { // exec 142a59b10
    P_FINISH(stack);
    auto pawn = ptr_at(self->_gap_0x8, 0x218);
    if (!pawn) return;
    auto klass = U_142a8ae70();
    if (!U_raw_class_check(pawn, klass)) return; // class+10/+38 and +30 table check
    Raw12 a = load12(DAT_148218630), b = load12(DAT_148218640);
    Word out = 0;
    if (!U_141a481a0(pawn, &a, &b, &out)) return;
    Word64 tag = 0;
    auto world = self->vfn_150();
    if (world && (world = U_141641350(world)) && U_14199e430(world))
        tag = *U_142cd4ab0(&tag, L"CreativeGameplay", 1);
    Offset selected = 0x3598;
    if (self->ResurrectionComponent && U_141b4d240()) selected = 0x35b0;
    if (byte_at(self->_gap_0x8, 0xe8) == 3)
        self->LevelStreamRequestHandshakeState[0] = 0;
    Raw12 b_copy = b, a_copy = a;
    U_142a53030(self, &a_copy, &b_copy, out, tag, self->vftable + selected);
}

void SetupClientRespawnTimerAndLocation(Controller* self, Frame* stack) { // exec 142a5d2d0
    // Raw stack layout contradicts generated argument labels. Each of first
    // three destinations occupies 12 bytes; last is a zero-initialized word.
    Raw12 first, second, third; Word last = 0;
    stack->StepCompiledIn(&first); stack->StepCompiledIn(&second);
    stack->StepCompiledIn(&third); stack->StepCompiledIn(&last);
    Raw12 first_copy = first, second_copy = second, third_copy = third;
    P_FINISH(stack);
    U_1419f5920(self, &first_copy, &second_copy, &third_copy, last != 0);
}

void CancelCreativeMatchmakingReadyCheck(Controller* self, Frame* stack) { // exec 142a56400
    P_FINISH(stack);
    auto world = self->vfn_150();
    if (!world) return;
    if (self->vfn_1c60()) {
        auto obj = U_14237f3b0(self);
        if (obj) {
            auto state = U_140d91b30(U_142290c00(obj));
            U_14226f840(state, 3);
            state = U_140d91b30(U_142290c00(obj));
            U_14226f940(state, 0);
        }
    }
    auto owner = ptr_at(world, 0x188);
    if (!owner) return;
    auto klass = U_1428ea7f0();
    auto actual_class = ptr_at(owner, 0x10);
    if (!U_raw_class_table_check(actual_class, klass)) return;
    auto subsystem = U_141e613c0();
    if (!subsystem) return;
    if (DAT_1480dffb0 > 4)
        FMsg_Logf_Internal(nullptr, 0, &DAT_1480dffb4, 5,
            L"[AFortPlayerControllerAthena::CancelCreativeMatchmakingReadyCheck] Ready Check Canceled!");
    U_140690620(subsystem + 0x378, self);
    U_1406a9dc0(self->InGameMatchmakingReadyCheckCanceled, 0);
}

void StartCreativeMatchmakingReadyCheck(Controller* self, Frame* stack) { // exec 142a5d460
    // First 16-byte local is zeroed and later freed; property labels conflict
    // with those operations. Second local is initialized only at byte zero.
    Raw16 first = {0}; Raw8 second; second.byte0 = 0;
    stack->StepCompiledIn(&first);
    stack->MostRecentPropertyAddress = nullptr;
    stack->StepCompiledIn(&second);
    auto address = stack->MostRecentPropertyAddress ? stack->MostRecentPropertyAddress : &second;
    P_FINISH(stack);
    U_1419fb550(self, &first, address);
    if (first.ptr) FMemory_Free(first.ptr);
}
