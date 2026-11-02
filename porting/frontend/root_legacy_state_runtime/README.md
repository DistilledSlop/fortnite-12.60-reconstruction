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
No generic RemoveFromParent shortcut is appropriate.

Remaining concrete requirements: root virtual480 and NUI virtual510/518 bodies,
byte constructors, exact native field order/guards, config singleton/selection,
layer cleanup and queue operations, deferred-removal producer/consumer/lifetime.
Do not emit unresolved virtuals or invent defaults. Includes for the transition:
UFortUIDataConfiguration, UFortUIStateWidget_NUI, notification/Toast types, SBox,
SNullWidget, UObjectGlobals and UEnum. No generated Native files were edited.
