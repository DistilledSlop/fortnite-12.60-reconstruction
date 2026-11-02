# Activatable widget route bridge

Generator fragments implement three real, non-pure route hooks and the Slate
release notification. The project-local hook and delegate names are descriptive,
not recovered original identifiers. Integrate `policy.apply`, `declarations`,
`definitions` and `validate_output`; copy the connected runtime's
`CommonActionRouteTypes.h` into CommonUI and join the reflected protected const
BlueprintImplementableEvent `UWidget* BP_GetDesiredFocusTarget() const` before
compiling. Existing activation delegates remain in common_panel_runtime.

Original image base 0x140000000, class block 145a004e0, widget vtable 1459ed638:
* +470 -> 1407c25c0 clears the outer optional's is-set byte only. The nested
  optional supports both inheritance and an explicitly empty inner value; its
  payload enum is unresolved and represented as uint8, not a guessed enum.
* +478 -> 1407c26c0 clears optional config is-set; the config's three bytes are
  ECommonInputMode, EMouseCaptureMode and bool, supplied by the connected route
  runtime. Both hooks are virtual so derived widgets can override the defaults.
* +480 -> 1407d1ee0 jumps to reflected event wrapper 140808b40.
* +260 -> 1407ee740 calls UserWidget ReleaseSlateResources 143d83b50 first,
  then broadcasts native multicast at widget+2c0 using 140684de0. The broadcast
  invokes with no argument. Child constructor 1407a9300 adds an SP callback to
  that multicast before constructing the child node; the widget is a bound
  payload to parent callback 1407c70c0. The callback removes the parent's delegate
  bindings from that widget and removes its first matching child node. Therefore
  use `AddSP(..., Widget)` and FSimpleMulticastDelegate, not a one-parameter
  broadcast or a synthetic release event.

`original-windows.json` records bounded source windows. These fragments are not
integrated into the generator or built here. Enum identity and actual derived
producers remain unresolved; retaining the byte avoids asserting an unsupported
identity while preserving both optional levels and overriding behavior.

## Rebuild discovery and construct

RebuildWidget +2a8 -> 1407e7c00 broadcasts `this` on process-global native
multicast 14782ee30 BEFORE calling UserWidget RebuildWidget 143d801e0. Its
consumer setup 1407cdf50 binds UObject callback 1407c6180, which compares
widget owning local player to router+20 and appends a weak widget to router+40
when equal. `CommonActivatableWidgetRebuilt.h/.cpp` expose this same dispatcher;
the router subscribes through its accessor. +2b0 is a different OnWidgetRebuilt
routine that re-associates action handles before its superclass, and is not the
source of this global broadcast.

NativeConstruct +318 -> 1407d1430 calls super first. When bIsBackHandler, it
resolves UCommonUIInputData CDO DefaultBackAction from CommonInputSettings,
constructs a binding descriptor with input mode/event zero and flags 0,1,1,
binds the widget's virtual back handler, and registers through router before
autoactivation. The inherited user-widget registration helper 1407ed3f0 has
the same router lookup, registration and handle append semantics. Then, when
bAutoActivate and not active, it calls the virtual activation request. The
back handler 1407d2c20 returns false if back handling is disabled; otherwise it
calls BP_OnHandleBackAction and, if unhandled and active, requests deactivation,
returning true. The generator must join the protected BlueprintImplementableEvent
`bool BP_OnHandleBackAction()` from reflection create 140805df0, flags 08080800.
