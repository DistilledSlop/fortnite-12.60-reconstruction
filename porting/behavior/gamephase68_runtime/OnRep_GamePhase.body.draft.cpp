// Behavioral draft of 0x14199ac00, reached from wrapper 0x1419a65b0.
// This is a reconstruction, not a compilable implementation. All Unresolved*
// operations represent missing native effects, never empty implementations.
// Raw offsets are from the original 12.60 image; names are not inferred APIs.

void AFortGameStateAthena::OnRep_GamePhase_body(EAthenaGamePhase OldGamePhase)
{
    const uint8 NewPhase = UnresolvedReadByte(*this, 0x1cb0);
    const uint8 OldPhase = static_cast<uint8>(OldGamePhase);
    if (NewPhase == OldPhase)
        return;

    // 0x14199ac00: log and crash/analytics context; Stub_Empty in the
    // decompile is an identified empty binary callee, not proof of wider semantics.
    UnresolvedLogGamePhase(UnresolvedPhaseName_141f90650(NewPhase));
    UnresolvedEnsureCrashlyticsChannelIfGlobalZero();
    UnresolvedContextEffect("GameContext", UnresolvedPhaseName_141f90650(NewPhase));
    if (NewPhase == 2) {
        ++UnresolvedGlobalGameCount_14811352c;
        UnresolvedGameCountContextEffect("GameCount");
    }

    UWorld* World = UnresolvedVirtualWorld_150(*this);
    auto* LocalContext = Unresolved_14463f750(UnresolvedGlobal_148351eb8, World);
    if (LocalContext) {
        World = UnresolvedVirtualWorld_150(*this);
        auto* Controller = Unresolved_14447f220(LocalContext, World);
        if (Controller && UnresolvedIsAthenaPlayerController(Controller)) {
            // Static labels initialized once: AthenaGlider, AthenaPickaxe,
            // AthenaCharacter, AthenaContrails, AthenaBackpack, Unknown.
            UnresolvedInitCosmeticContextLabels();
            UnresolvedCosmeticRecord Record{};
            if (!UnresolvedControllerComponentValid_5a8(Controller)) {
                auto Source = Unresolved_14237c740(Controller);
                Unresolved_141984b70(Record, Source);
                UnresolvedDestroySource_1409f5080(Source);
            } else {
                Unresolved_140a003b0(Record, UnresolvedControllerComponentValue_5a8(Controller));
            }
            // Original invokes vtable +0x3e0 for non-null slots, otherwise
            // uses "Unknown", and calls the empty binary helper after each.
            UnresolvedCosmeticContextEffect(Record, 0x28, "Unknown");
            UnresolvedCosmeticContextEffect(Record, 0x30, "Unknown");
            UnresolvedCosmeticContextEffect(Record, 0x40, "Unknown");
            UnresolvedCosmeticContextEffect(Record, 0x20, "Unknown");
            UnresolvedCosmeticContextEffect(Record, 0x68, "Unknown");
            UnresolvedDestroyCosmeticRecord(Record);
        }
    }

    auto Value = UnresolvedVirtual_920(*this);
    Unresolved_141eaee70(*this, Value);
    const uint8 Role = UnresolvedReadByte(*this, 0xf0);
    if (NewPhase >= 1 && NewPhase <= 3) {
        // Both actor iterators use flags 5 and the same world/object filtering.
        for (auto* Actor : UnresolvedFilteredActorRange_14192cca0(
                 UnresolvedVirtualWorld_150(*this), UnresolvedClass_142a82010(), 5))
            UnresolvedActorVirtual_918(Actor);

        if (Role == 3 && NewPhase == 3) {
            for (auto* Actor : UnresolvedFilteredActorRange_14197f280(
                     UnresolvedVirtualWorld_150(*this), UnresolvedClass_142752f80(), 5)) {
                if (Unresolved_141baf5f0(Actor))
                    UnresolvedActorVirtual_868(Actor, false);
                else if (UnresolvedActorFlag(Actor, 0x6ac, 0x10) &&
                         !UnresolvedActorFlag(Actor, 0x6a9, 0x04))
                    UnresolvedActorVirtual_8b0(Actor);
            }
        } else {
            for (auto* Actor : UnresolvedFilteredActorRange_140894360(
                     UnresolvedVirtualWorld_150(*this), UnresolvedClass_142752f80(), 5)) {
                if (UnresolvedActorFlag(Actor, 0x6ac, 0x10) &&
                    !UnresolvedActorFlag(Actor, 0x6a9, 0x04))
                    UnresolvedActorVirtual_8b0(Actor);
            }
        }
    }

    if (NewPhase == 2) {
        if (Role == 3 && Unresolved_141a4a530(UnresolvedPreferredPointer(*this, 0x1df8, 0x1df0))) {
            for (auto Weak : UnresolvedWorldControllers_14468fb70(UnresolvedVirtualWorld_150(*this)))
                if (auto* Controller = UnresolvedResolveAthenaController(Weak))
                    Unresolved_1419fc560(Controller); // original call has implicit argument registers
        }
        UnresolvedResetGlobals_1481590c8(); // +0, +8, +c, +14, +1c zeroed
        if (Role == 3) {
            for (auto Weak : UnresolvedWorldControllers_14468fb70(UnresolvedVirtualWorld_150(*this))) {
                auto* Controller = UnresolvedResolveAthenaController(Weak);
                if (!Controller) continue;
                UnresolvedCosmeticRecord Record{};
                if (UnresolvedControllerComponentValid_5a8(Controller)) {
                    UnresolvedCopyCosmeticRecord(Record, UnresolvedControllerComponentValue_5a8(Controller));
                    Unresolved_141d425e0(Controller, 9, L"InGame_Warmup", 0, Record);
                    // Two additional initially empty output strings passed on stack.
                }
                UnresolvedDestroyCosmeticRecord(Record);
            }
        }
        if (auto* Object = UnresolvedReadPointer(*this, 0x1ec0))
            Unresolved_144229900(UnresolvedVirtualWorld_150(*this), Object);
    } else if (OldPhase == 2) {
        if (Role == 3 && Unresolved_141a4a530(UnresolvedPreferredPointer(*this, 0x1df8, 0x1df0))) {
            for (auto Weak : UnresolvedWorldControllers_14468fb70(UnresolvedVirtualWorld_150(*this))) {
                auto* Controller = UnresolvedResolveAthenaController(Weak);
                if (Controller && UnresolvedControllerByteAtUObjectIndex122_NamePlus1(*Controller) != 0)
                    Unresolved_1419fd000(Controller, true);
            }
        }
        if (auto* Object = UnresolvedReadPointer(*this, 0x1ec0))
            Unresolved_1442251f0(UnresolvedVirtualWorld_150(*this), Object);
        if (Role == 3)
            Unresolved_1419ae490(*this);
    }

    if (NewPhase == 3) {
        if (Role < 3) Unresolved_1419a9580(*this);
        UnresolvedResetGlobals_1478c2fc0(); // includes two FLT_MAX fields and 0x141c8d330
        auto* Global = UnresolvedGlobal_148351eb8;
        if (!Global || !Unresolved_14197e310(Global)) Global = nullptr;
        Unresolved_141e7e7a0(Global, true, false);
        Unresolved_141988940(*this);
        Unresolved_1419a8ec0(*this);
    }
    if (Role == 3 && NewPhase == 2)
        UnresolvedWritePointer(*this, 0x1400, Unresolved_142ca95c0());
    if (NewPhase == 5) {
        auto* PhaseWorld = UnresolvedVirtualWorld_150(*this);
        if (PhaseWorld && UnresolvedReadPointer(*PhaseWorld, 0xf0) && Unresolved_14419d570() &&
            Role == 3 && UnresolvedRuntimeOptionsByte_0bbb())
            Unresolved_1419b59e0(*this, UnresolvedReadPointer(*PhaseWorld, 0xf0));
    }
    Unresolved_141e9a1a0(*this);
    auto PhaseText = UnresolvedVirtual_0a70(*this);
    Unresolved_141eb30f0(*this, PhaseText);
    Unresolved_142bce820(L"AthenaGamePhase", PhaseText);
    LocalContext = Unresolved_14463f750(UnresolvedGlobal_148351eb8, UnresolvedVirtualWorld_150(*this));
    if (LocalContext && Unresolved_14197e350(LocalContext) &&
        (OldPhase == 1 || OldPhase == 2) && !(NewPhase == 1 || NewPhase == 2))
        Unresolved_142649d20(UnresolvedAddress(*LocalContext, 0x448), UnresolvedVirtualWorld_150(*this));
    UnresolvedBroadcast_1406a9dc0(UnresolvedAddress(*this, 0x1240), NewPhase);
    UnresolvedDestroyPhaseText(PhaseText);
}
