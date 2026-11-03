# Native message handlers: rough runtime lift

These are connected behavioral lifts of the actual native bodies, not forwarding wrappers. `rough.cpp` names message copies, callbacks, timer state and refcount operations while retaining the observed branches, cleanup, offsets, and live reads. It is an integration draft, not a verified ABI-safe implementation.

Source SHA-256 inventory:

- `porting/generated/native-broad74/0x14199a400.c`: `949f4e87a30da138bfccbb2f5cdc8d2a5a522fd0cc6298e493cc6a3732fe9e98`
- `porting/generated/native-broad74/0x14199a760.c`: `70a16190a66cb6fab5c46654a7c07ca01ce5ede30623c7cdbd110d24707bc582`

Address inventory:

- `0x14199a400`: HandleContextTutorialGameModeMessageRequest; positive delay obtains context at virtual +0x150, constructs a delegate through `0x141981b60`/`0x14197b750`, schedules through `0x1446278d0`, and cleans both delegate representations. Near-zero path queues via `0x14198a860` at owner +0x10c8. Negative delay skips dispatch. All paths release the incoming shared owner.
- `0x14199a760`: HandleGameModeMessageRequest; positive delay copies and retains the message, allocates a 0x70-byte callback and sets vtables `0x145e13800` and `0x145e13858`, captures `0x142bad740`, schedules through `0x1446278d0`, then cleans delegate, allocation and copied owner. Near-zero path queues through `0x14198aa30` at owner +0x10b8. Negative delay skips dispatch. All paths release the incoming shared owner.
- Other direct addresses: `0x14198abf0` callback; `0x142ee3b30` initialization; `0x145803c70` stack cookie check; `0x1480c4288` cookie global; `FMemory_Realloc` and `FMemory_Free` allocator effects. Virtual calls retain their literal vtable offsets.

Unresolved integration boundaries: extern declarations are placeholders for native ABI and effect adapters, not implementations or authoritative signatures. Atomic increment and decrement adapters must implement the original lock-prefixed 32-bit operations, including previous-value return semantics for decrement. The decompiler's disjoint stack variables can represent overlapping native call frames, including scheduling options and callback state; recover stack layout and calling convention from assembly before executable integration. In particular the stack cookie, delegate lifecycle, indirect calls, and apparent uninitialized stack fields require ABI reconstruction; no default values were invented. The read/write, virtual call, allocator, queue, callback-building and timer scheduling functions are individually declared engine adapters and have no fabricated implementations. TimerScratch and MessageCopy are logical field containers, not claims about native layout; the scheduling adapter must marshal the tutorial request argument and stack options into the original outgoing frame. Stack cookie and native code/vtable address resolution remain explicit adapters. The source cleanup path where the tutorial builder count is nonzero and its pointer is null skips the builder free; it is preserved as observed.

No compiler, test, Ghidra, generation, or commit was run for this bounded task.
