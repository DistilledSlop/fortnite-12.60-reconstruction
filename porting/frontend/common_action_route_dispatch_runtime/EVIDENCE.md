# Connected action dispatch source

Generator inputs only; the primary agent owns promotion, Native/Engine changes,
combined builds, validation and commits. This directory uses the existing
`CommonActionBindingData::Find` map and the actual record layout. It introduces
no binding registry, reflected replacement type or numerical enum fallback.

Original mapped image:
`/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
mapped base `0x140000000`. Bounded disassembly was obtained with:
`python3 /mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py VA:HEX_SIZE`.
All reads used bounded seeks; no full-image reads or hash operations.

Recovered boundaries:

| Original | Source |
| --- | --- |
| base vtable1459ecdd0 +08 =1407e35b0 | ProcessHoldInput |
| node/root vtables1459ecdf8/1459ece30 +08 =1407e39c0 | children-first ProcessHoldInput |
| base +10 =1407e4290 | ProcessNormalInput |
| node/root +10 =1407e4440 | children-first ProcessNormalInput |
| base +20 =1407d0660; node +20 =1407d0830 | CanReceiveInput |
| 1407adc50 | NativeCommonActionCanReceiveInput |
| 140802a10 | UpdateHold |
| 1407ac590 | existing ProgressListeners.Broadcast |
| 1407e3b40 | NativeProcessActionInput |
| 14079e590 | NativeDispatchNormalActionInput |
| 1407ada10 | NativeShouldBlockActionInput |
| 1407ffd30 tail1407ffec8–1407fffa4 | full queued repeat dispatch and true return |

Disassembly windows included `1407e35b0:480`, `1407e3a10:160`,
`1407ffd30:650`, `140802a10:280`, `1407ac590:150`,
`1407d0660:1d0`, `1407d0830:220`, `1407adc50:240`,
`1407e3b40:300`, `1407e4290:260`, `14079e590:180`,
`1407e44d8:ec`, `1407ada10:1e0`, `1407adc03:4d`,
`143008800:160`, `143008640:1b0`, `1407866d0:100`.

The hold result bytes are witnessed as 0 handled, 1 synthesize press, 2
unhandled; the shared route types declare descriptive names for these values.
Short release generates press only when elapsed <=0.25 and elapsed < hold time.
The 0.25 float is at1459a86ec; platform time offset16777216 double at1459a8710.
Pending hold state is cleared on exhaustion even after non-consuming matches.
Consuming hold matches return their current result immediately, including
unhandled for in-progress repeats; no invented handled result was added.

Queue operations are AddUnique for pressed/repeat and ordered removal for
release. Tick does not empty the queue. It calls the standalone route first,
then the enabled active root only for unhandled. The discovery agent provides
the real pending-widget/root-selection prefix called by the complete ticker.

The normal continuation dispatches synthetic pressed and then the actual
event, with the actual event's result taking precedence. `1407ada10` defaults
true except menu mode with an existing Slate user, where it returns viewport
cursor capture. `143008800`/`143008640` match the existing Engine
FSlateUser::DoesWidgetHaveCursorCapture/DoesWidgetHaveCapture source, not focus.

Slate eligibility checks enabled, visible and children-hit-test-visible, then
the parent's public ValidatePathToChild virtual, then the first matching
FCommonButtonMetaData's actual weak button and NativeIsInteractionEnabled.
The metadata name was read at1459f0630; weak button is metadata+08. Parent owns
the shared CommonButtonMetaData.h and real button rebuild producer. Base route
checks every ancestor; a node stops before its cached Slate boundary and also
accepts exhausting the parent chain, matching1407d09ce.

Integration: compile CommonActionBindingRoute.dispatch.cpp beside the existing
CommonActionBindingRoute.cpp; append router includes and body fragments. Method
declarations and enum/storage were coordinated with the hierarchy agent in
common_action_route_connected_runtime. No per-helper test/build was run; this
is a source-reviewed component pending the primary's coherent batch build.

The bounded analog-cursor task is separately owned by common_analog_cursor_runtime.
Factory1407d12a0 creates the FAnalogCursor-derived object via1407896b0,
vtable1459fc448 and router raw owner+50; its full behavior is not represented
by a fake default IInputProcessor here.
