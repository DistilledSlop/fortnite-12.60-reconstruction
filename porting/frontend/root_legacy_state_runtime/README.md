# Legacy root state transition candidate

Unregistered source, no build/runtime claim. Original body 14516f380:257
selects the config class, allows virtual480 to mutate selector and class, records
pending state, deactivates the previous legacy widget and clears layers only when
that previous widget exists. It creates the new NUI widget with the owning player,
commits the pending selector, attaches content, activates with the previous state,
handles state9 notification wiring and retires the old widget. Preserve callback
ordering and member re-reads. Config provider source lives in the sibling
startup_class_providers_runtime directory.

145174c00:e9 is the real SBox SetContent operation using the already reconstructed
NativeRootBox2e8. Its parameter accepts UUserWidget, so both legacy and new state
paths can use one helper after joining the existing startup declaration.
14514e200:166 caches a FindObject enum lookup for EFortUIState (literal146e45e18)
and returns its value name, or Unknown (1459a4b20). Transition log text is at
146e45d80. All evidence was read in bounded ranges.

145174fa0:14b queues a weak widget reference at FortUIDirector+b8, then calls
the equivalent of MarkPendingKill (internal object flag20000000). The subsystem
getter1453113f0 names FortUIDirector at literal14700ed32. The removal fragment
is a lifetime candidate: its native weak array declaration, processing tick,
lifetime and ownership must be joined before promotion. The removal
log literal146e46290 is UI Director is trashing widget [%s], verbosity6.
Both diagnostic fragments retain unresolved log bindings until the actual
category identity is established; no guessed category is emitted. Transition
verbosity is4, with the exact literal above. Logger providers must preserve
their original verbosity guards before evaluating name conversions.
No generic RemoveFromParent shortcut is appropriate.

Additional bounded joins: root ctor1452c69a0:b0 installs vtable146fc7bf0 and
zeroes both selector bytes in one word store at1452c69f2. Base slot480 points
to1406801c0 (ret0); Athena vtable146fc80c8 overrides it with14514ace0, which
can replace selector3 with4 after real controller/party/game-instance operations.
That override is not supplied. NUI ctor145143d00:68 installs146e386e0; its
510/518 entries are145160ac0/145160ec0, forwarding to Blueprint ProcessEvent
wrappers1453181c0/145318200. Deactivation receives the newly selected byte in
DL (loaded14516f3da); activation receives the previous state. Both exact reflected
event identities must be joined before implementing those wrappers.

Remaining concrete requirements: Athena virtual480 and NUI virtual510/518 bodies,
exact native field order/guards, config singleton/selection,
layer cleanup and queue operations, deferred-removal producer/consumer/lifetime.
Do not emit unresolved virtuals or invent defaults. Includes for the transition:
UFortUIDataConfiguration, UFortUIStateWidget_NUI, notification/Toast types, SBox,
SNullWidget, UObjectGlobals and UEnum. No generated Native files were edited.
