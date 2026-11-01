# GI Init recovered from the later original capture

The new original-memory image closes the previously protected GI Init frontier.
It proves a fourth direct InitializeUI caller: **GI Init itself**, in its
nondedicated branch. It also proves both real map-delegate registrations and
matched Shutdown removal. This is source evidence; no lifecycle implementation
or runtime acceptance is claimed.

## Input and analysis boundary

The input is `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
165,991,936 bytes, SHA-256
`338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`.
VA equals `0x140000000 + fileoffset`. The capture's separate receipt pins the
original execution and memory-read tools. This investigation neither executed
the client nor accessed its process; it only read the finished image.

The earlier image/proof remains valid for its narrower snapshot. In particular,
its formerly protected **14987c5b1** is now executable:
`48 8d 4d 50 48 89 87 10 04 00 00 50 5b e9 30 b8`.
The new bytes store the constructed McpProfileManager at GI+410 and continue
through **141e67df3**. No old raw image or shared Ghidra program was overwritten.

Ignored evidence is `generated/frontend-native-evidence/late-gi-init-recovery-1`.
The primary `proof.json` SHA-256 is
`e9fae0fe8e7b3d61fe766440c76f3497872aa057a91d9d326e30affbd812fdcf`.
It freezes 1,225 merged original code/data windows, all decoded reachable Init
instructions, all proven dispatcher chains and the exact graph. The receipt
also freezes decompiler outputs, actually read sources, tools/dependencies and
fresh replays/tests.

## Closed control flow

The installed FortGI table **145fef1a0**, Init slot **280**, selects **141e67d50**.
Its primary unwind span remains `[141e67d50,141e6a1b7)`, with executable gateways
outside that span. Reachability recovers **5,702 original instructions**, including
**149 synthetic-return dispatcher chains**, and **334 call instructions**.
After treating each proven dispatcher as its actual branch, there are **4,037
instructions including dispatcher entries** and zero unknown control frontiers.
The sole exit is **141e109aa**, `jmp qword ptr [rsp-8]`, following the original
nonvolatile-register/frame restoration; this consumes the saved caller return
logically, rather than introducing an unknown game-code successor. A separate graph-wide
stack analysis proves identical stack depth at every merge and exactly +8 bytes
at that exit on every reachable path, consistent with the Win64 call ABI.

The ordinary disassembler initially appeared to finish after 98 instructions:
**149665db5 RET** is actually a conditional branch. The proof evaluates every
identified dispatcher for both condition outcomes, verifies every general
register and the stack pointer are restored, and checks the exact selected
address. Allowed instructions preserve flags. These are bounded machine-state
proofs of branch equivalence, not guesses based on nearby movabs constants.

Structured decompilation used a NEW isolated raw-image Ghidra project under
`/tmp/late-gi-init-recovery-1/ghidra`. Its input is a separately generated
analysis-only copy: each proven dispatcher entry jumps to a direct branch or
a synthetic conditional-branch stub. Original instructions/windows and all
149 replacement descriptions remain available independently. The synthetic
stubs are explicitly outside the original image at **14a000000**. Normalization
never touches the original image, process, shared project, Native or Engine.
All program modifications were rolled back; the raw import project itself is
new scratch analysis, not an updated shared analysis database.

`init-decompiled.c` contains the resulting complete intraprocedural body. Its
Ghidra-generated names/prototypes are provisional: omitted inferred register
arguments in a decompile are not proof that an original C++ method is parameterless.
The original instruction graph remains authoritative.

## Authentic UI entry and balanced map handling

The source order in the nondedicated branch is:

1. Construct/initialize the original client services and highlight configuration.
2. Add static **1420fcf00** to the GI+5a8 native delegate.
3. Bind **141e728e0** with this GI to global **147c4e8b0**, through original
   AddUObject helper **1406a81a0**, at the original **141e69343** call.
4. Call **InitializeUI 141e6c120**, at **141e6934b**.
5. Continue the original Pegasus/replay/configuration work; this call does not
   terminate Init or replace the remainder.

The later unconditional Init path binds **141e72150** to global **147c4e8c8**,
constructing an original weak-UObject delegate instance and adding it at
**141e698xx**. The exact member-function LEA is **1495c3d91** → **141e72150**.
The PreLoadMap member-function LEA is **140930381** → **141e728e0**.

The globals are **FCoreUObjectDelegates::PreLoadMap** and
**FCoreUObjectDelegates::PostLoadMapWithWorld**, respectively. Original Engine
LoadMap **144646d30** broadcasts the first with URL.Map and the second with the
loaded world or nullptr on failure. The complete original bodies/assembly are
frozen. Public Engine UnrealEngine.cpp independently corroborates that paired
lifetime and UObjectGlobals.h declares the exact argument types:

```cpp
void UFortGameInstance::Init();
void UFortGameInstance::Shutdown();
void UFortGameInstance::InitializeUI();
void UFortGameInstance::OnPreLoadMap(const FString& MapName); // 141e728e0
void UFortGameInstance::OnPostLoadMap(UWorld* LoadedWorld);   // 141e72150
```

`OnPreLoadMap` is a descriptive source name for the proven callback; its original
C++ spelling is not recovered. OnPostLoadMap is additionally corroborated by
its original logging literal. Original private field **688** is a signed pending
load counter; its exact source member name is not recovered.

PreLoadMap checks IsDedicatedServerInstance; for a client it performs an original
postincrement, and only when the old count is below 1 obtains director
**141856620** and calls its **288** slot for each GI local player. Preserve this
slot separately from InitializeUI's **290/280** sequence.

PostLoadMap, for a client, writes `max(counter-1,0)`. If the result is below 1,
it calls **141e6c120** before further world/controller/analytics work. A null
LoadedWorld does not prevent that gated InitializeUI call. The rest of the
callback is substantial and is frozen in full; reducing it to the counter and
UI call would drop original behavior.

**Shutdown 141e82360**, installed at GI table slot **288**, removes this GI from
both globals via **140690620** (`RemoveAll(this)`). It also removes the GI from
the current world's native delegate at **world+5f8** before other teardown.
No added blanket removal from unrelated delegates is inferred. Preserve the
recovered complete Shutdown service/subsystem cleanup.

## Construction and other registrations

Fresh reflection decoding from this late image binds the actual reflected
members. Getter literals independently establish constructed class identities:

| GI offset | Reflected member | Constructed original class |
| --- | --- | --- |
| 410 | ProfileManager | McpProfileManager |
| 738 | UpdateManager | FortUpdateManager |
| 9c0 | GameFrameworkComponentManager | GameFrameworkComponentManager |
| 758 | ChatRoom | FortChat |
| 7b0 | MobilePushNotificationManager | FortPushNotificationManager |
| 7a0 | SeasonalEventManager | FortSeasonalEventManager, original platform predicate guarded |
| 7a8 | TournamentManager | FortTournamentManager |
| a10 | ReplayVideoManager | ReplayVideoManager |
| 6b8 | TooltipManager | FortTooltipManager, client branch |
| 740 | InteractabilityTracker | FortInteractabilityTracker, client branch |
| 430/438 | Matchmaking/MatchmakingV2 | FortMatchmaking/FortMatchmakingV2, client branch |
| 440/448 | RejoinCheck/SocialManager | FortRejoinCheck/FortSocialManager, client branch |
| 450 | MatchAnalytics | FortMatchClientAnalytics for client; FortMatchServerAnalytics for dedicated |
| 458 | PartySpectateAnalytics | FortPartySpectateAnalytics, client branch |
| 9b8 | MaterialCacheManager | FortMaterialCacheManager, later nondedicated branch |

The full Init also installs shared Slate input state while preserving previous
manager pointers and configures the complete named highlight collection. It
retains Replay/Pegasus/Extraction command-line paths. These remain part of the
method, rather than unrelated functions accidentally collected by an unwind scan.
Exact getter/field/data outputs are in `class-identities.json`,
`late-metadata.json` and `init-literals.json`.

Additional observed registrations remain separately typed work:

- UpdateManager+40 receives weak-UObject callback **141e70c30**. Its input is
  byte-sized in the original body; the exact enum name is not proven here.
- Actual **FortAssetManager**, obtained by **141df62a0**, has its +758 native
  event bound to **141e70480**. That callback validates an opaque result/context,
  routes equal/pending mode bytes **8b0/8b1** through **141e52bb0/141e664e0**, and
  optionally calls **141e505b0**. Preserve its additional arguments and behavior.
- GI+818 shared service+10 receives **141e717e0**, an encryption-key processing
  callback; its final source class/event name needs direct type attribution.
- Static callbacks **141e855d0/141e77680** are installed once using global
  guard **14812e684** into **147ce04e8/147ce0500**. They process region selection.
- If engine-loop completion byte **148247289** is clear, GI binds **141e72110**
  into **147c324c8**; otherwise it invokes it immediately. The callback's full
  feature-status/broadcast behavior is retained.
- GI byte **a20** is cleared and callback **141e72de0** is registered into
  **147c32600**; that callback writes its boolean argument to a20. Exact public
  delegate identity is not established by that store alone.
- Current world+5f8 receives **141e64a80**. Its argument is tested as
  FortGameState_InGame/FrontEnd and it updates AssetManager state only in mode6.
  Do not name the event solely from its offset.

## Preserved gates and precise residual boundary

The initial direct InitializeUI call uses the complete existing method, which
still returns for a missing world, GI mode bytes3/2/4, startup phase below3 or
the configured Kairos map comparisons. Startup phase helper **141ed7750** reads
**FortAssetManager+6cd**, after original owner/class validation. It is not an
arbitrary startup flag. This proof supplies no reason to force phase3/4, enable
legacy input or replace real player/account hooks with a registry insertion.

The protected-byte frontier is resolved and the complete **intraprocedural Init
CFG is recovered**. The **transitive source implementation is not complete**:
private service-event parameter types, actual lower-level player/account hook
bodies, several service method identities and their complete source integration
remain required. Original bytecode is available in the finished image; no new
original execution is required to continue those tasks. The typed map binding
and direct gated call are implementation-ready seams, not a license to replace
all remaining Init work with those few lines. All four product milestones stay
false.

## Replay

```sh
python3 -I porting/frontend/inspect_late_gi_init_control_flow.py \
  --image /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin \
  --proof porting/generated/frontend-native-evidence/late-gi-init-recovery-1/proof.json \
  --output /tmp/fresh-late-gi-init-replay.json
python3 -I porting/frontend/test_inspect_late_gi_init_control_flow.py
```

Replay uses only the Python standard library. It verifies the whole original
image/proof hashes, exact original windows/instructions, both outcomes of all
149 dispatchers, every direct branch/call displacement, graph closure and the
concrete Init/PreLoadMap/PostLoadMap callback addresses. Eight tests include wrong
destination, lost restore, flags-changing operation proof-mutation and unbalanced-epilogue rejection.
The decompiler is producer evidence; replay does not execute Ghidra or a client.
