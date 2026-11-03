# Building navigation batch 50

Two proposed reflected ABuildingActor bodies preserve the conditional bit writes and subsequent native navigation refresh. `0x1448f9ce0` matches `UNavigationSystemV1::UpdateActorInNavOctree`: its early static-navigation guard, cast to the navigation-relevant interface, actor world lookup, navigation-system type check, and default octree update match `/opt/fortnite/UnrealEngine/Engine/Source/Runtime/NavigationSystem/Private/NavigationSystem.cpp` (function at line 2438). The native receives the actor in RCX through the tail jump. The exported static engine API is declared in `NavigationSystem.h`; integration needs the NavigationSystem module dependency and header. The callback occurs only when the respective bit changes, and after the write.

Native body and reflected wrapper byte windows, plus the navigation callee, are pinned in `native-evidence.json`. Signatures and reflected parameter names are in `manifest-entries.json`; wrapper addresses occupy `address`, and implementations occupy `native_body_address`.

Excluded: `SetBRDrawDistances` writes offsets 0x478 and 0x47c and dispatches virtual slot 0x8b0, whose actual callback has not been identified. `SetUsesDayPhaseChange` at 0x141bc5220 changes a bit and performs extensive conditional world/delegate work; it is not closed by this evidence. Neither callback nor branch was replaced or omitted in a proposed body.

No canonical files were edited and no tests, build, or runtime execution were performed. The native disassembly and generation scripts are in `/tmp/building-nav50`.
