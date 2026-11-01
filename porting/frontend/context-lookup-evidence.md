# Original BlueprintContext lookup

The original `UBlueprintContextLibrary::GetContext` resolves a local player and
supports two engine subsystem families before consulting a separate context
registry. The generated `UBlueprintContextBase` derives from `USubsystem`, so its
Fortnite contexts cannot be faithfully implemented by assuming every requested
class is a game-instance subsystem. Registry construction and lifetime still need
recovery before porting this helper.

`inspect_context_accessors.py` verifies the original memory-image SHA-256,
uses its RVA-based exception directory to bound each function, and preserves raw
bytes and direct objdump disassembly. Seven static class accessors load the literals
below into RDX and call the same `GetPrivateStaticClassBody` target
`0x142d698b0`. These are original executable class literals, not names inferred
from current engine offsets.

| Accessor | Original class literal | Use |
| --- | --- | --- |
| `0x143dbe060` | `UserWidget` | Context object selection |
| `0x1447bb440` | `PlayerController` | Context object selection |
| `0x144786700` | `LocalPlayer` | Context object selection |
| `0x1447867c0` | `LocalPlayerSubsystem` | First requested-class branch |
| `0x14470a080` | `GameInstanceSubsystem` | Second requested-class branch |
| `0x144801560` | `Subsystem` | Requested-class validation |
| `0x1408232c0` | `BlueprintContextBase` | Custom registry class-key validation |

The input is
`generated/dump/FortniteClient-Win64-Shipping.runtime.bin`, SHA-256
`604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d`.
It is a memory image with stale PE section raw offsets: **VA = `0x140000000` +
file offset**. The tool deliberately uses RVAs directly. Fresh proof is under
`generated/frontend-native-evidence/context-accessor-identities-2/receipt.json`.
The first run with six engine-class identities remains preserved separately.

## Observed lookup behavior

The 376-byte body at `0x140822310` checks a non-null context object in this order:

1. A `UserWidget` calls virtual slot `+0x290` to obtain the player candidate.
2. A `PlayerController` reads its `+0x290` member, accepting a `LocalPlayer`.
3. A `LocalPlayer` is accepted directly.
4. Otherwise, or if no candidate resulted, it uses the engine world-context
   helper and a first-entry lookup. The precise field and helper names in this
   fallback have not been independently identified here.

With a player candidate, it calls `0x1442f3c00`, obtains the shared manager through
`0x140822100`, and invokes the 751-byte lookup at `0x140822490` with that player
and the requested class. The existing decompile of `0x1442f3c00` follows members
`+0x70`, then `+0x80`, returning null if the first is absent. Naming it
`ULocalPlayer::GetGameInstance` is consistent with the current engine's viewport
chain and the subsequent subsystem branch; the member names are an inference.

In `0x140822490`, a requested subclass of `LocalPlayerSubsystem` is looked up
through `0x14460d300` at player `+0xd8`. A requested subclass of
`GameInstanceSubsystem` uses the object from `0x1442f3c00`, then collection
`+0xd0`. Each successful result is checked against its respective subsystem
class. These operations agree with the current engine's subsystem collection
accessors, but the original offsets must not be copied into rebuilt classes.

The final branch uses manager storage `+0x20`. It builds a two-word player key,
looks up a player entry of stride `0x60`, then looks up the requested class in
that entry's storage at `+8` (class-entry stride `0x18`) and returns its object
at `+8`. Either reported `INDEX_NONE` produces null. The player-key helper
`0x1408219d0` hashes and compares both 32-bit key words. The class-key helper
`0x140821b10` hashes the class pointer, validates both candidate and requested
keys against `BlueprintContextBase`, then compares the validated pointers
exactly. It walks hash collisions rather than constructing missing contexts.

The 998-byte manager constructor `0x14081fca0` initializes the player map and
another map at `+0x78`, stores a weak reference to its supplied object at `+0x70`,
and subscribes raw callbacks `0x140822a30` / `0x140822c00` to globals
`0x147ce0788` / `0x147ce07b8`, retaining handles at `+0xc8` / `+0xd0`.
The first callback inserts an object's name and weak reference into the second
map and schedules a bound `0x140822910` callback through a helper. The other
checks an engine lookup against the weak owner and invokes an iteration helper
with callback `0x1408206c0`. The original delegate names, complete callback
semantics, player/context insertion, GC ownership and teardown are still
unfinished; these callbacks must not be guessed to be local-player events.
The five fresh decompiles are in
`generated/frontend-native-evidence/context-manager-decomp-1/`, with disclosed
Ghidra bookkeeping changes and no saved program. The direct-disassembly tool
also snapshots these bounded original functions without opening Ghidra.

## Reproduction and limits

From the repository root:

```sh
python3 -I porting/frontend/inspect_context_accessors.py \
  --out porting/generated/frontend-native-evidence/context-accessor-identities-2
```

Use a fresh output directory for subsequent runs. The command requires the
pinned local original image and objdump, leaves them unchanged, and does not
open or modify the Ghidra project. Existing function decompiles remain under
`generated/frontend-native-evidence/{fortnite-frontend-bodies,get-context-deps}/`.

This evidence identifies routing and narrows the missing registry work. No
`GetContext` implementation, context creation, widget construction, interactive
menu, or runtime parity is established by this static inspection.
