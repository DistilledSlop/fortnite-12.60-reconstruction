# Aircraft unlock setter integration proposal

The fragments implement 0x1419adfe0 and the observed default virtual bodies.
Names of the setter, its two native int32 fields, and the team-count virtual are
reconstructed, not recovered original spellings. The unsigned addition preserves
32-bit machine wraparound without signed-overflow undefined behavior.

**Integration gates:**

* The helper generator currently emits nonvirtual members. Add an explicitly
  validated virtual-helper policy before promoting the two virtual helpers.
  Emit `virtual AFortTeamInfo* GetPlayerTeam()` on AFortPlayerState and
  `virtual int32 GetNativeTeamMemberCount()` on AFortTeamInfo. Calls must retain
  C++ virtual dispatch, including for future recovered derived overrides. The
  exact original cv qualifiers cannot be determined from these machine bodies;
  these proposed non-const declarations are an internally consistent contract.
  The available native class-parent map lists AFortTeamInfo and
  AFortTeamInfoAthena as the only native classes deriving from AFortTeamInfo;
  their +0x630 slots both point to 0x141f60e80. This does not establish that
  every possible runtime/Blueprint subclass cannot override a native virtual.
  AFortPlayerState and AFortPlayerStateAthena primary vtable slots +0x770
  both point to the PlayerTeam getter 0x1424192a0; other player-state
  subclasses have not been audited for overrides. Do not replace
  either call with a direct property read at the call site.
* The selected UE implementation of GetPlayerControllerIterator returns an
  indexed iterator over PlayerControllerList (World.cpp:4371, World.h:73),
  whose condition rechecks IsValidIndex against the array length (Array.h:119).
  The original iterator constructor uses World+0x1c8 and index zero; independently
  establishing this is PlayerControllerList in the original world layout remains
  open. Until verified this is a candidate, not a closed source body.
* The native fields use offsets only for declaration ordering: generated classes
  do not generally retain the original binary layout. The names describe the
  observed writes only, not a proven full semantic contract. Don't expose or
  replicate them without independent evidence.
* Calling GetWorld without a null guard is intentional. After iteration the
  spectator pointer and each source field are read in program order as shown.
  No OnRep or event is invoked by this setter.

See `/tmp/gameplay-aircraft-unlock/proposed-manifest.json` for concrete entries.
The field and setter entries may be integrated with the normal helper schema;
virtual-helper entries require generator changes as stated above. Do not mark
this proposal ported or claim build/runtime validation until the gates are closed.

## Closed integration gates

The generator now supports boolean `virtual` on member helpers and verifies the exact virtual declaration; static helpers and non-boolean flags are rejected. Primary independently identified original `UGameplayStatics.GetPlayerController`: reflection/native_funcs.json names exec `0x14470f700`; it calls `0x144215e10`, whose world/controller-index algorithm directly calls `0x14468fb70`. The algorithm matches Engine GameplayStatics.cpp GetPlayerController (world-context resolution, player-index iteration, weak Get, signed bounds/count reload and return/null). This independently identifies the latter as GetPlayerControllerIterator, hence world+0x1c8 is PlayerControllerList. Raw disassembly argv and windows are in `/tmp/gameplay-aircraft-unlock/get-player-controller-{exec,body}.{bin,asm}`.

Raw window 0x14470f700, 0xbd bytes SHA-256 `8e411933b08f3b8c9696cf66718c2476cdcc68634a909deb9be055bdaddf8c36`.

Raw window 0x144215e10, 0x7c bytes SHA-256 `f48a14eba17d326fdea3257f4e593e2fc78c2319759f6023d4a43d9164adb60d`.
