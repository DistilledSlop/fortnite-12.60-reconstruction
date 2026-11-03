# Legacy Panel operation queue source candidate

This directory implements the bounded CommonInputManager Panel queue from
`../common_input_manager_operation_queue.md` and the five completion bodies in
`../common_activatable_panel_native_events.md`. It is intentionally unregistered,
unpromoted and unbuilt. No Native, Engine, stage or generated reflection file was
modified. Root owns integration and combined client/flow validation.

Implemented: full 0x28 Operation layout with unreflected FGuid at 0x14; predicted
stack; the two exact AddCleanOp cancellation rules; public Push/Pop and processing
suspension; direct versus transitioned Intro/Outro/Push; pre-counted and removed
Pop prefix (resetting outstanding count before counting); five weak UObject callbacks capturing GUID by value; native manager
multicasts at original +30/+48/+60/+90; original reset and three-tick suspension
tail. The real Panel held-action map/progress cleanup, activation subscriptions,
native and dynamic notification ordering come from common_panel_runtime.

GUIDs are not completion guards in the original. Frozen callback bodies use them
only for log formatting. They unbind the *current* single-cast field and complete
without any GUID comparison. This implementation preserves the payload but omits
the verbose trace messages; adding rejection by current-ID would change behavior.
Synchronous reentry remains direct; no processing loop or deferred completion is
substituted. Unexpected consumed records retain original return/stall behavior.

Panel stack byte304 is added as bNativeInActivationStack. Virtual hooks +4E8/+4F0
remain real calls before Added and after Removed Blueprint events respectively.
Their default empty bodies are proven, not placeholders: a bounded read from
original-late-capture-1/image-125.runtime.bin at 0x1459edff8 of 16 bytes yielded
`c001684001000000c001684001000000` (both 0x1406801c0); a 3-byte read there yielded
`c20000`. The already recorded constructor table base is 0x1459edb10.

Integration hooks:

- Apply policy after original manager suspension and Panel intro/outro policies,
  passing actual native exec map. The owner/signature/property/type gates fail
  closed against original-policy.json, copied from current frozen reflection.
- Call validate_types(gate), header_prelude, declarations, definitions and
  emit_headers. Call struct_declarations inside FOperation and struct_postlude
  after its complete declaration. The latter enforces all field offsets, size
  and alignment at C++ compile time. Keep existing manager suspension members;
  queue uses exactly those two state flags, not duplicate flags.
- Source currently requires no new module dependency. Existing CommonInput panel
  dependency remains mandatory. Existing Panel friend UCommonInputManager grants
  access to private callbacks and protected events/handlers.
- NativeInputSuspensionChanged is the single source-original manager+90 bool
  multicast. Lifecycle must bind the real weak subsystem callback to its dynamic
  OnInputSuspensionChanged and remove it during disable/deinitialize. Do not
  broadcast combined fd||fe here. Stack multicast source names are descriptive.

Remaining boundary: actual tick registration and held-handler tick(+28) belong to
the manager lifecycle/held dispatch component. NativeTickOperationSuspension()
implements the entire original fd branch and returns true even on the invocation
that clears fd. The *real* manager Tick must call it, return when true, and only
when false and both CurrentlyHeldActionInputHandler words exist invoke the actual
interface held-action tick with DeltaTime. No empty interface method, guessed
vtable cast, ticker or pretend handler is supplied. Thus this source does not
claim a complete executable manager Tick. NativeResetPanelOperations is the real
reset helper but its lifecycle caller must also be recovered/wired. Legacy manager
creation must continue to honor the selected FortUIManager policy rather than
forcing it on. Held-action dispatch/row registration and nonlegacy routing remain
outside this task.

Validation: `python3 -m py_compile
porting/frontend/legacy_panel_operation_queue_runtime/policy.py` PASS. Reviewed
against existing bounded frozen callback/reset/tick source and queue evidence;
no separate helper probes or UE build performed. Root should batch compile/real
queue transitions with lifecycle integration, especially synchronous native
completion, delayed Blueprint EndOutro, delegate reentry and the resume tail.

Final bounded review additionally checked frozen 1407fc000.c and 1407ffc50.asm:
pop dispatch resets +F8 before preparing the prefix, and tick compares the
32-bit counter using signed JLE after INC. Source preserves both details.
Policy application using both actual signature snapshots PASS (4 manager methods,
2 Panel events); no generation was invoked.
