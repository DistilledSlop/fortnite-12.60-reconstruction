# Codex backend handoff

Updated 2026-11-13 05:11 UTC. Work paused at the user's request to wind down.
The full backend objective is **not complete**. Do not substitute passing service
tests for original-client acceptance or native gameplay integration.

## Checkout, ownership and working preferences

- Main checkout: `/opt/fortnite/FortniteClient-12.60-CL-13477524`.
  Private origin: `DistilledSlop/fortnite-12.60-reconstruction`, branch `main`.
- Latest backend implementation is committed and pushed: **985fadb7**,
  `Grant free Yacht challenge and serve native competition download`.
  Backend and launcher paths were clean after that commit, before this handoff.
- There are substantial unrelated native/tool/dashboard edits in the shared
  worktree. Preserve them. Stage exact owned paths; never reset the worktree.
  The concurrent native owner controls native promotion, generation and builds.
  Do not start Claude workers: current repository instructions keep them stopped.
- Read root `AGENTS.md`, `porting/README.md`, this handoff, and the relevant
  `porting/server/README.md` / `EVIDENCE.md` sections before resuming.
  Some older README bullets describe failures later resolved in EVIDENCE.md.
- User prefers the primary agent to plan, orchestrate and review, while doing
  difficult implementation/debugging directly when useful. Delegate bounded tasks
  with explicit file ownership. Use `fork_turns: none`: Luna medium for small
  read-only tasks; Terra medium for regular implementation/recovery; Sol medium
  for hard recovery. At most three internal workers at once. Rotate workers at
  sensible task boundaries rather than assigning an entire milestone to one.
- Keep searches/output bounded; use `rg` first. Never search/print the huge
  single-line `native_funcs.json`. No repetitive full logs or broad reruns without
  changed risk. No native builds for backend-only changes.
- User deferred gifting. The requested historical shop date is the 12.60 release
  day, **2020-05-20**. Use DXVK for tests that need rendered 3D; the UI-only Wine
  mode intentionally has black 3D backgrounds. No new work is authorized merely
  by this handoff: wait for the user's request to resume.

## Validated baseline and recent implementation

Latest combined backend suite: **421 tests passed in 23.910 seconds**.

```bash
python3 -W ignore::ResourceWarning -m unittest discover \
  -s porting/server/tests -p 'test_*.py'
```

Private log: `porting/generated/deadpool-schedule-grant/backend-tests.log`.
No tests/builds are needed merely for this handoff documentation.

Recent pushed commits, oldest first:

- **65709cd4**: Agent Peely frontend quests and owned challenge schedule binding.
- **4c4180c1**: all registered Client objectives; verified original challenge map.
- **985fadb7**: free Yacht entitlement and native competition-download contract.

Current registry has **324 individual quest definitions in twelve broad groups**:
weekly 100, dailies 62, Location Domination 60, Deadpool 21, faction 15, Maya 9,
Storm the Agency 6, Oro 4, golden styles 6, XP coins 35, hidden fishing 3, Peely 3.
This is not the complete cooked quest inventory. Yacht was already in Deadpool's
21 definitions; fixing its grant did not increase the count.

The blank original challenge screen was fixed by reconciling owned schedule
`Granted_Bundles` arrays, in addition to bundle `GrantedQuestInstanceIds`.
Both contain actual owned item GUIDs with valid parent links. Malformed arrays
reject the transaction. Original DXVK acceptance on frozen 65709cd4 observed
21 owned bindings, 89 hydrated quest references and five visible categories.
The original challenge table displayed Storm the Agency quests/counters/rewards/
map markers and Location Domination staged quests and navigation. Captures:
`porting/generated/chrono-schedule-binding-probe-retry/`.

`UpdateQuestClientObjectives` now handles all twenty source-registered Client
objectives: fourteen Deadpool, three Midas, three Peely. Native wire shape is
`{"advance":[{"statName":"<lowercase BackendName>","count":1,"timestampOffset":0}]}`.
Use BackendName, not stat-table row names. Batches validate before mutation;
only owned Active objectives progress; eligibility is captured before the batch
so newly granted successors wait for a later request. Combat spoofing is rejected.
Native room-interaction delivery is still unverified. Midas has no invented grant.

The three free Peely quests use the cooked day-43 LobbySeason12 gate and reward
`AthenaDance:Emoji_S12_BananaAgent`. Source/hash metadata is in
`season12_peely_data.py` and `season12_client_objective_data.py`.

Yacht now has its separate cooked free schedule grant, day-46 quest activation,
trusted Visit progress, bundle counters and durable Freestylin' Remix music reward.
Main Deadpool remains a paid Battle Pass level-one schedule reward; do not make
it free or replay all paid rewards to repair an artificial test fixture.
Implementation: `season12_yacht_data.py`, `season12_grants.py`, `quest_service.py`.

`events_service.py` now answers authenticated owner-only
`GET /api/v1/events/Fortnite/download/{accountId}` with the native structure:
player identity plus empty tokens/persistentScores/pendingPayouts/teams and empty
events/templates/scores. The local competition calendar has no configured events.
This does not implement tournaments, scoring or historical event schedules.
Native contract: custom reader 0x140b22a40; player reader 0x140b26710;
download method 0x140ac0fc0; HTTP completion 0x140ac1980.
Private recovery: `porting/generated/events-download-contract/`.

## Other established backend behavior and remaining acceptance

- Shop/local currency, locker slots/styles/presets, Battle Pass rewards,
  durable accounts/profiles and many progression paths have existing tests and
  original-client probe evidence. See server docs plus `porting/LOCKER_*_PROBE.md`
  and `porting/PURCHASE_PROBE.md`; avoid reopening already accepted flows blindly.
- Maya hairstyle and custom scarf finalization, and banner selection, were
  accepted through the original UI and survived backend/client restart. Maya
  tests used artificial earned keys; gameplay-earned keys and other choices remain
  open. Quest reward popup presentation is not established by service tests.
- Two original clients have accepted native party ping-join with captain
  confirmation, promotion, ready-state propagation, friend whispers, party MUC
  messages and captain kick. See the later two-client acceptance in EVIDENCE.md
  around the native ping/confirmation section; older README statements are stale.
  Reconnect, expiry/rejection variants, full-JID multi-resource UI and voice remain
  unverified or incomplete. Voice conferences are unimplemented; suggestions are
  empty. Parties are process-local; do not invent a persistence requirement.
- Recent-player UI worked with explicitly seeded diagnostic match results.
  Actual native encounter/result delivery is pending. The registered local
  game-server HTTP/WebSocket loadout/result/quest-event bridge is tested, but the
  native server still must submit real validated results and qualified stat rows.
  Raw gameplay tag/filter evaluation is not implemented by accepting row names.
- Configured calendar anchors are explicit local policies. Placeholder event ends
  can yield absurd native countdowns. Historical calendar parity is not complete.
- ProjectChrono launcher, public-CA/TLS helper path, screenshots/wiki work and
  `porting/local_client/run_wine.sh` already exist. The Wine helper path retains
  TLS verification but changes in-memory pins/other launch boundaries. Strict
  unpatched G11 and native Windows acceptance remain open. Do not describe the
  diagnostic AppES callback as proven full entitlement behavior.
- Real payments, receipts, campaign/homebase and complete historical content remain
  incomplete. Gifting UI was explicitly deferred by the user.

## Next concrete quest work: Feats/Career

Read-only recovery found an omitted free source grant at
AthenaSeason12 `SeasonGrantsToEveryone[0]`:
`ChallengeBundleSchedule:Season12_Feat_BundleSchedule`.
It grants `Season12_Feat_Bundle` immediately. The bundle contains **107 distinct
quests**, none in the current 324: eleven `GrantWithBundle`, 96 `Manually`.
Do not grant all 107 just because their definitions exist.

Private reports:

- `porting/generated/season12-feat-contract/{schedule,bundle,normalized}.json`:
  all quest references/unlocks, career bit shifts, schedule/bundle hashes.
- `porting/generated/season12-feat-contract/grant_with_bundle_definitions.json`:
  the eleven initial definitions, objectives, serialized fields and hashes.
- `porting/generated/career-feat-profile-contract/findings.json`:
  newly identified native completion-storage type and exact string addresses.
- Season entitlement report:
  `porting/generated/faction-vtid-review/AthenaSeason12.json`.

The eleven initial quests use ItemEvent objectives, not stat-table rows:
eight `HasLeveledUpAthenaSeason` quests (110, 250 and six gold thresholds
140/180/250/260/300/350), two `HasAccumulatedItem` weekly bundle-token thresholds
1/10, and a hidden Location Domination token threshold 20. No direct quest reward
links are serialized in these eleven. Do not confuse these career feats with the
already implemented golden-style unlock quests or invent inherited rewards.

The Feat bundle tags include `Store.CompletionTimes` and
`DeleteQuestsOnCompletion`. Existing generic quest logic retains claimed items;
blindly applying it could lose Career parity or break reward/repair receipts.
Important discovery: a separate **UFortChallengeBundleProgressToken** has exact
reflected fields `bundle_template`, `BundleItemDef`, `completion_bits: int32[]`
and `completion_times: int32[]`. Native readers, primary template identifier,
bit packing, timestamp semantics and MCP serialization still need recovery.
The original Athena `EndBattleRoyaleGame` signature includes `GrantedQuestDefs`;
manual-feat grant authority/rules are not yet established. Nothing from this Feat
recovery was added to tracked backend implementation.

Resume by recovering that small completion-token/grant contract, then implement
a coherent bounded Feat path with durable completion history, legitimate grants
and integration with existing level/token events. Avoid declaring the family
complete after only granting a schedule. Other omitted free season grants include
the Season11 named-location and landmark discovery quests; asset age alone does
not invalidate their explicit Season12 entitlement.

Working property-report tool (default compiled Cue4Inspect is older):

```bash
dotnet exec \
  --runtimeconfig porting/cue4inspect/bin/Release/net10.0/Cue4Inspect.runtimeconfig.json \
  porting/generated/season12-agent-quest-reports/tool/bin/Debug/net10.0/Cue4Inspect.dll \
  FortniteGame/Content/ASSET_PATH /mnt/vibrato/fortnite-extracted-full \
  --property-report --loose
```

Do not add `--depsfile`: it can select the older assembly and reject this mode.
Raw recovered packages/reports and runtime logs remain ignored, outside Git.
Never open the shared Ghidra project; use a private clone only when necessary
and delete it afterward. Original flat runtime image:
`/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
base VA `0x140000000`.

## Native probe state at shutdown

**No Codex-owned backend/client probe or recovery worker remains running.**
All listed original sessions are terminal; owned processes are gone and the
`g1-isolated-probe/wine-prefix` is idle. No debugger is attached. Do not kill
unrelated Wine/native processes or assume unrelated concurrent work should stop.

- `chrono-native-frontend-objectives-probe`: frozen 4c4180c1, session 96990,
  backend 348544; normal 600-second run ended. Rendered Battle Pass room at
  1280x1024, five successful native MCP requests. Agent controls appeared clipped;
  no native objective updates, all three Peely quests Active/zero. Of 69 requests,
  67 were 2xx; content-controls and then-missing events download were 404.
- `chrono-events-frontend-wide-probe`: frozen **985fadb7**, session 34784,
  backend 358972/client 359082. Original rendered mode picker at **1600x900**.
  Stopped deliberately on the user's wind-down request before advancing modes.
  45 requests: 44 were 2xx, known tolerated content-controls 404; two native
  QueryProfile requests. **No event download occurred**, so original-client
  acceptance of the new event route remains unverified. No native room objective
  acceptance was established. This supplies no regression evidence.

Each directory under `porting/generated/` contains `result.json`, source
provenance, isolated state, request logs and screenshot. The wider probe uses a
private Xvfb `-screen 0 1600x900x24` wrapper around the unchanged tracked DXVK
launcher script. Use a widescreen viewport for the next actual room interaction
test; the last probe does not prove that widening alone fixes the controls.

On resume, start a new owned probe only after checking the prefix remains idle.
Keep backend revision/state provenance, wait for helper readiness before any
debugger attachment, and poll the original session handle to terminal. An
observation timeout is not a stopped process and is not a reason to relaunch.
Prior diagnostic fixture manually marks paid rewards claimed and may lack the
main Deadpool schedule; that is not evidence its real paid grant is broken.
