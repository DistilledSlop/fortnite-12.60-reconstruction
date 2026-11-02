# CommonButton SetStyle candidate

This is an unregistered, unbuilt source candidate, not a production receiver.
`definitions.inc` assumes the integrated `NativeRebuildStyle` and the real
`TSubclassOf` behavior in the target UE source. It is deliberately not wired to
the generator: production initialization and its receiver graph are incomplete.

Original function block `0x1478342c0`, outer `0x140805cb0`, create
`0x1408074a0`, flags `0x4020401`: `void SetStyle(TSubclassOf<UCommonButtonStyle>
InStyle)`. Native registration entry `0x145a05260` joins the name to exec
`0x14080d6f0`; exec calls receiver `0x1407f9c70`. The receiver rejects null or
wrong-ancestry incoming class, compares separately typed old and incoming values,
returns if equal, stores the incoming class and calls rebuild `0x1407acd30`.
Null does not clear Style. Rebuild can synchronously call the text-style event,
which can reenter the setter after the store; do not cache state across rebuild.
There is no default-style selection in this receiver: the incoming class is the
selected value; rebuild reads its CDO without creating one and returns if absent.

Rebuild is already supplied by `../common_button_style_runtime/definitions.inc`:
it creates the normal, selected and disabled style caches, selects current style
(selected first, then enabled/disabled), sends the text-style event and refreshes
minimum dimensions with fresh post-event CDO and weak-object reads. Its dimension
operation unconditionally dereferences the independently resolved weak internal
button. `NativeInternalButton` has no production writer in the current tree.
Binding the setter now risks the exact real viewport path, not merely a synthetic
case; no null guard or assignment-only fallback is faithful.

The original producer `Initialize` `0x1407cd7d0` first calls its parent; on
success it constructs `InternalRootButton` in WidgetTree, copies click/touch/
press/focus and enabled/interaction state, stores the weak pointer, and, only if
an old root exists, attaches it via a zero-padding fill ButtonSlot, replaces the
root and binds five callbacks in order: clicked, native double-click, native focus,
pressed, released. Parent reentry precedes the weak store, and the null-root
branch lacks that strong containment/binding. Coherent integration still needs
all five complete receiver paths, selection, focus/hover and action routing,
parent and full lifecycle/reentry/GC behavior. Some individual callback helpers
have since been integrated; their existence does not close the graph. See
`/mnt/vibrato/fortnite-work/common-button-initialize-closure-1/HANDOFF.md` and
`/mnt/vibrato/fortnite-work/button-initialize-dependency-map-1/HANDOFF.md` for
original branches and remaining dependencies. Never install a partial producer.

Once lifecycle is closed, generator integration must validate the full original
function metadata and native-exec join above, owner and `Style` declaration, reject
collisions, require the style-rebuild policy and complete lifecycle policy before
adding a BlueprintCallable reflected function and the definition, and validate
rendered output before writes. Compile and exercise real widget initialization,
class rejection/equality, event reentry and size propagation in the coherent GUI
batch. No claim of compiled or runtime validation is made here.

Bounded verification on 2026-11-02: inspected
`PYTHONPATH=/tmp/common-input-capstone python3 /mnt/vibrato/fortnite-work/common-button-initialize-closure-1/range.py 0x1407f9c70 0x1407f9d20`
and the adjacent `0x1407f9d20 0x1407f9d50` window; inspected the exact original
reflection block and UE `Templates/SubclassOf.h` operator*/Get/assignment. The
original join and broader setter proof are in
`/mnt/vibrato/fortnite-work/common-button-set-style-contract-1/HANDOFF.md`.
No full image read/hash, generation, build or runtime run was performed.
