# Root layer clear — bounded source candidate

The root virtual 0x498 points to 0x14515b410. Its translated body is in
`definitions.inc`; `declarations.inc` is the virtual declaration already
present in startup_root_added_runtime/declarations.json. The root body is not
linkable until the real provider declarations in RootLayerClearProviders.h are
implemented and joined. Do not substitute empty definitions or skip those calls.
No generator, Native, Engine or build changes are included.

The original first resolves UFortMcpContext through 0x1425dc880 using this root
as world context; if non-null, 0x1421cb230 examines its unreflected pointer at
0x378 and invokes 0x1411ce2d0 only when present. That helper checks its +0x50
and passes the literal `UserCancelled` to 0x1411d0c30 when non-null. The latter
has substantive task/state transitions, so these two context providers remain
unresolved. UFortMcpContext identity is joined by its class registration
0x142990750 and block 0x146433050, not inferred from the `UserCancelled` text.

The root next calls virtual 0x270 with enum byte 1 on reflected
ProgressModal_ControllerDisconnected (+0x328), then 0x1407b7ed0. Virtual 0x270
is SetVisibility and byte 1 is ESlateVisibility::Collapsed; 0x1407b7ed0 invokes
the deactivation virtual only when the panel's active byte is set, matching the
already recovered UCommonActivatableWidget::DeactivateWidget. The original
makes no null check. Native root then calls 0x1407ae7f0 on reflected
WidgetQueue_ModalContent (+0x310), which tail-calls 0x1407f9d70 with zero index
and false animation. That function checks its Slate switcher, removes its
active child's deactivation bindings, deactivates the child or sets pending
state and calls the switcher transition 0x140800440. It is not an array clear
and remains an unresolved provider.

The next instruction calls 0x14514c4e0 with the address of reflected
ConfirmationQueue (+0x280) and slack zero. The helper destructs all entries
of original size 0x118 and empties the array; reflection resolves its element
as FFortDialogDescription_NUI (0x142b25370, size 280). `Empty()` supplies the
native destruction and zero slack semantics when the generated struct has its
proper fields/destructors.

Finally, the root invokes 0x144f664f0 on reflected
ConfirmationWindow_ConfirmationWidget (+0x318) and 0x144d74df0 on reflected
ErrorWindow_Errors (+0x320), unconditionally. The former tests 0x143d66ef0,
copies its dialog state, invokes 0x144f75f90 and 0x144f55ae0 and potentially
notifies the global UI context. The latter hides the panel, selects an outro or
deactivation path, copies a 0x70-byte entry array, invokes live delegates and
empties it. Their owner types are UFortConfirmationWindow and UFortErrorWindow;
these nontrivial providers remain unresolved, including their native state and
callback identities. The root body therefore represents exact ordered wiring,
not a completed connected implementation.

Includes needed: RootLayerClearProviders.h, UFortProgressModal.h,
UCommonActivatableWidgetQueue.h, UFortConfirmationWindow.h, UFortErrorWindow.h,
and the generated dialog struct. The MCP helper and provider declarations must
be linked from their actual owning modules. No build or runtime validation was
performed. Image reads were bounded to listed entry points and narrow supporting
ranges; no whole-image read or digest was performed.

`modal-queue.definitions.inc` additionally translates the complete immediate
0x1407f9d70 container branch and its 0x1407ae7f0 caller. The remaining lower
boundary is the original custom Slate switcher at container +0x1c8: its
0x1407c19a0 accessor may first refresh a cached index from a bound attribute,
and 0x140800440 stores the requested widget weakly, handles an existing
animation's direction/reversal, may start a new curve and active timer, changes
the Slate switcher index, and broadcasts transition state/index delegates.
The queue's displayed-widget delegate removal and deactivation must happen
before that transition. Merely replacing this lower boundary with the engine's
plain SWidgetSwitcher SetActiveWidgetIndex would discard original animation and
delegate behavior. The fragment declares abstract accessor methods as unresolved
integration contracts, not virtual no-op implementations. It requires the
actual shared switcher object and its construction/lifetime to be recovered.
