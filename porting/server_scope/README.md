# Which native functions does a server need?

Classifies each of the 14,391 in-scope native functions (the denominator of `gameplay.native_behavior`) by whether a
dedicated server for the unmodified original client needs it. Read-only analysis; nothing here is a port.

## Tiers

| Tier | Meaning | Evidence |
|---|---|---|
| R1 | Required | Called from Blueprint bytecode in the content a server loads (Apollo map, Athena game mode, state, controller, pawn, playlists, and everything they reference, with client-only packages pruned) |
| R2 | Required | `Server` RPC on a class that content instantiates; the original client can send it |
| R5 | Required | Reached by direct calls, through helpers, from the body of an R1 or R2 function |
| L1 | Likely | Declared `BlueprintAuthorityOnly` on an instantiated class; server-side, but nothing shows it is called |
| L2 | Likely | `NetMulticast` on an instantiated class; also runs on the server when it calls it |
| N1 | Not required | Class derives from an engine class a dedicated server never instantiates (`UWidget`, `AHUD`, `ULocalPlayerSubsystem`, `UGameUserSettings`, `UCheatManager`, ...) |
| N2 | Not required | `Client` RPC: the body runs on the owning client; the server only sends |
| N3 | Not required | Declared `BlueprintCosmetic`: not run on dedicated servers |
| N4 | Not required | `OnRep_*` RepNotify, unless server code calls it directly (those become R5) |
| P1 | Probably not required | A plain callable function that no Blueprint anywhere calls, no native code calls, and no vtable or data table points to |
| P2 | Probably not required | As P1, but called only from Blueprints the server does not load |
| U1 | Undetermined | Class is instantiated by server content, but nothing shows this function is used |
| U2 | Undetermined | No server-closure content references the class. It may still be spawned from native code |

P1 and P2 are weaker than N1-N4. Calls inlined by the compiler leave no reference, so absence of a reference is evidence, not proof. Events and delegate handlers (`BlueprintEvent`, names starting `On`, `Handle`, `Notify`, `Event`, `Callback`, `Trigger`, `Received`) are never judged unreferenced, because ProcessEvent reaches them by name.

Undetermined means undetermined, not unneeded. The current direct-call analysis cannot close it: much dispatch in this
binary is virtual or goes through `ProcessEvent`, so that graph adds only dozens of functions (R5). Runtime coverage is
also required: record the UFunctions the reconstructed server executes while the original client plays. Those traces
cover exercised paths; they do not prove unexecuted branches or cover every non-reflected native helper.

## Network contract coverage

Generate the current recovered native declaration inventory with:

```sh
python3 -I porting/tools/network_contract_inventory.py
```

The default output is `porting/generated/agents/network-contract-inventory/`:
`declarations.jsonl`, `summary.json`, and `issues.json`. It joins original class
identities, RPC roles and parameters, replicated fields and conditions, RepNotifies,
and available source/generation evidence. Source hashes and the Git commit identify
the inputs. Unknown identities, conditions, and conflicting declarations are
reported explicitly.

This is a declaration inventory, **not a complete expected interaction list**.
It includes the engine and other modules present in the recovered metadata;
inherited declarations appear at their declaring owner. Its counts therefore
differ from the older native-set authority inventory. Blueprint declarations,
backend exchanges, native behavior missing from the metadata, and execution order
are not completely covered. A manifest entry or matching class checksum is not
runtime verification.

Behavioral coverage must connect each action and state to the received request,
authority checks and mutations, outgoing RPCs/replication, client consumers, and
expected observations. Owner and remote observers, late joins, streaming readiness,
equipment changes, and delayed or lost messages need separate scenario evidence.
The runtime-required ledger planned in `SERVER_FIRST_PLAN.md` WP B1/B3 is still
incomplete; the declaration inventory does not close that work.

## Verdict

`functions.jsonl` gives every function a verdict (`v`) and a confidence (`conf`):

| Verdict | Functions | Ported | Confidence |
|---|---:|---:|---|
| required | 7,222 | 3,055 | proven 1,244, strong 57, likely 380, unresolved 5,541 |
| not required | 7,169 | 1,345 | proven 4,228, strong 845, probable 2,096 |

Unresolved functions default to required: nothing shows a server can do without them, and a wrongly dropped function
breaks a match while a wrongly kept one only costs effort. The 1,681 required or likely functions are the floor, and the
7,222 are the ceiling.

## Reproduce

```sh
python3 -I porting/server_scope/scan_content_refs.py /mnt/vibrato/fortnite-extracted-full/FortniteGame/Content refs.jsonl 80
python3 -I porting/server_scope/engine_parents.py /opt/fortnite/UnrealEngine engine_parents.json
objdump -d -M intel --no-show-raw-insn porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe > full.asm
python3 -I porting/server_scope/callgraph.py porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe edges.json full.asm extra.json
python3 -I porting/server_scope/data_refs.py porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe porting/generated/thunk-table/final3.json data_refs.json
python3 -I porting/server_scope/classify.py refs.jsonl engine_parents.json porting/server_scope --edges edges.json --data-refs data_refs.json
```

`extra.json` is the list of thunk and body addresses from `generated/thunk-table/final3.json`. Outputs:
`server_scope.json` (counts per tier, with how many already have a committed body) and `functions.jsonl` (one row per
function: class, name, tier, ported, UFunction flags).

## Limits

- Content evidence over-approximates: a Blueprint's import table lists a function even when the call is on a client-only branch.
- Pruning rules (`PRUNE_PATHS`) drop effects, sound, HUD and UI packages from the closure; `--no-prune` changes R1 by about 30.
- `.pdata` end addresses in the dump are unreliable, so a function extends to the next function start.
- R5 ignores shared stubs (one body serving several functions) and helpers called from 300 or more places.
- Tried and rejected: counting callers of each class's `StaticClass()`. Live and dead classes overlap too much (about a third of known-live classes look unreferenced), so it was not used.
- Native-only spawns (no content reference) are invisible to the content evidence and land in U2.
