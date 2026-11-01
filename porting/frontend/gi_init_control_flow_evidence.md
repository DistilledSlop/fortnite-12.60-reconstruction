# Original GI Init frontier and FortniteUI registration

The static proof recovers the reachable Init prefix and binds the third known
InitializeUI caller to **FortniteUI::PostLoadCallback**, the module reload hook.
It does not recover the complete Init body or bind OnPostLoadMap registration.
No source lifecycle implementation or runtime result is claimed.

Evidence lives in new ignored `generated/frontend-native-evidence/authentic-gi-init-control-flow-1`.
Its receipt freezes the source inspector, probe scripts/dependencies, original
windows, complete Ghidra outputs, prior documentation actually read and replays.
The original memory image SHA-256 is
`604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d`;
VA is `0x140000000 + fileoffset`. The normalized runtime EXE SHA-256 is
`5f056080237b5a0fdab284700d5bd975acf8c30ffeeb37472f5ba5542fa2dc38`.

## Reachable Init prefix

The installed GI table's Init entry `141e67d50` reaches `1492b711a`, whose
prologue saves nonvolatile registers, reserves `3d8` stack bytes and records the
original `STAT_Inv1PerfHook_FGI_I` provider. Its direct jumps reach:

`141e67d8a → 141e67d44 → 14426cdf8 → 141e67d92 → 148e6286a → 141e67dab → 1411477d4 → 141e67db7 → 14918788d → 141061391 → 1409f6809 → 1493f3afb → 141e67de0 → 14987c5b1`.

There are **71 decoded reachable instructions, 15 direct jumps and 5 calls**.
Calls are base GameInstance Init `14421c740`, dedicated-instance predicate
`14421ee60`, original NewObject construction guard `142ec0440`, original
McpProfileManager class getter `141111280` and construction `142ed7360`.
The superclass identification is corroborated by full original body and pinned
public Engine GameInstance.cpp: session construction/delegates, encryption
callbacks, then subsystem collection initialization. No UI event binding is
visible in this recovered prefix.

At `14987c5b1` bytes begin `96 9d 11 3b d5 65 08 9b 05 0d ab da e5 d1 5b b2`.
Capstone decodes xchg/popflags/adc and stops on invalid long-mode opcode at
`14987c5b5`. This is an **undecodable/protected frontier**. Do not treat those
initial decoded instructions as valid source semantics or invent a successor.
A complete static CFG cannot be claimed from this pinned snapshot.

The original exception record gives the primary Init span
`[141e67d50,141e6a1b7)`, unwind descriptor `1474d2848`. It does not cover either
obfuscated gateway. The proof freezes this entire primary span, but an unwind
span is not a complete reachable CFG or a license to include adjacent code.

## Exact third-caller ownership

Original static initializer **1405dd3c0** passes the literal **FortniteUI** and
factory **144dc7e30** into the original module-registration structure. The
factory allocates `28` bytes and installs **146d71770**. Its module slots are:

| Offset | Original body | IModuleInterface operation |
| --- | --- | --- |
| 0 | 144da17e0 | destructor |
| 8 | 144da0800 | StartupModule |
| 10 | 144da0aa0 | PreUnloadCallback |
| 18 | 144da09b0 | PostLoadCallback |
| 20 | 144da0be0 | ShutdownModule |
| 28/30/38 | 1406801d0 | true-return module flags |

The eight-slot correspondence is pinned to Engine ModuleInterface.h. Public
ModuleManager.cpp calls PostLoadCallback after LoadModule in
LoadModuleWithCallback, supporting its reload meaning. The original module
slot, factory, registration literal and full hook body provide the binding;
the public source does not replace unavailable original instructions.

PostLoadCallback invokes module slots68/78, visits the current controller path,
then obtains GameEngine's GI and invokes InitializeUI when it is FortGI.
This is **not ordinary initial StartupModule → InitializeUI**. Its preceding
operations remain part of the original hook.

StartupModule loads CommonUI and EpicCMSUIFramework, binds CMS callback
144db0ad0, and installs a shared Slate input manager via **143031d60**. That
setter only exchanges the input-manager shared pointer at Slate offset5e8,
matching public FSlateApplication::SetInputManager. The installed input object
uses the **separate four-entry table146d71750**, immediately before the module
table. Treating both as one vtable misattributes the UI entry callback.
ShutdownModule installs the original default input table146654338. These full
bodies are frozen; no reduced replacement is proposed.

## Remaining implementation boundary

The full-image byte scan finds no conventional pointer, RIP LEA or direct
relative reference to OnPostLoadMap 141e72150; a separate low32-pointer scan also
finds none. This does **not** establish that callback's unreachability or that
its delegate need not be registered. The other caller 141e664e0 has direct
relative candidates at 141e5290d/141e6e410/141e704b7; earlier complete bodies
remain the authority for their semantics.

The exact missing original input is executable code at **14987c5b1** and its
successors during GI Init, sufficient to recover the remaining event/delegate
bindings and their cleanup. A separately disclosed original execution/capture
would be needed if no independently authorized decoded snapshot exists. This
investigation did not execute the original client or modify process memory.
Do not force InitializeUI, startup phase3/4, or replace GI/LocalPlayer methods
with registry calls. Keep AssetManager progression, actual player/account
callbacks and native UI hooks intact when source-complete implementations are
later integrated.

## Replay and integrity

```sh
python3 -I porting/frontend/inspect_gi_init_control_flow.py \
  --image porting/generated/dump/FortniteClient-Win64-Shipping.runtime.bin \
  --proof porting/generated/frontend-native-evidence/authentic-gi-init-control-flow-1/proof.json \
  --output /tmp/fresh-gi-init-replay.json
```

The replay verifies the whole memory-image hash, 1,506 exact original windows,
all 20 direct call/jump edges, exact module slots and protected frontier. It does
not run Ghidra. Producer probes used Ghidra only after a process check found no
live job. Six read-only probe batches never saved/renamed the program; each
records the five expected project bookkeeping-file changes. No missing
functions were defined and no program transaction was committed. No
Native/Engine/stage/generator source or current runtime phase was changed.
All four product milestones remain false.
