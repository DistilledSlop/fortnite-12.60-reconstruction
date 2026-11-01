# Original BlueprintContext lookup

The original `UBlueprintContextLibrary::GetContext` resolves a local player and
supports two engine subsystem families before consulting a separate context
registry. The generated `UBlueprintContextBase` derives from `USubsystem`, so its
Fortnite contexts cannot be faithfully implemented by assuming every requested
class is a game-instance subsystem. Registry construction and lifetime still need
recovery before porting this helper.

`inspect_context_accessors.py` verifies the original memory-image SHA-256,
uses its RVA-based exception directory to bound each function, and preserves raw
bytes and direct objdump disassembly. Six static class accessors load the literals
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

The input is
`generated/dump/FortniteClient-Win64-Shipping.runtime.bin`, SHA-256
`604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d`.
It is a memory image with stale PE section raw offsets: **VA = `0x140000000` +
file offset**. The tool deliberately uses RVAs directly. Fresh proof is under
`generated/frontend-native-evidence/context-accessor-identities-1/receipt.json`.

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
at `+8`. Either reported `INDEX_NONE` produces null. The helper implementations,
key lifetime, registration, and teardown remain unfinished. The manager
constructor at `0x14081fca0`, the two lookup helpers `0x1408219d0` and
`0x140821b10`, and their writers are concrete next targets.

## Reproduction and limits

From the repository root:

```sh
python3 -I porting/frontend/inspect_context_accessors.py \
  --out porting/generated/frontend-native-evidence/context-accessor-identities-1
```

Use a fresh output directory for subsequent runs. The command requires the
pinned local original image and objdump, leaves them unchanged, and does not
open or modify the Ghidra project. Existing function decompiles remain under
`generated/frontend-native-evidence/{fortnite-frontend-bodies,get-context-deps}/`.

This evidence identifies routing and narrows the missing registry work. No
`GetContext` implementation, context creation, widget construction, interactive
menu, or runtime parity is established by this static inspection.
