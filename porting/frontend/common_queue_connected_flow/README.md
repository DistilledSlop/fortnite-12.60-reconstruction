# Queue connected flow diagnostic

Copy `QueueConnectedFlow.h` and `.cpp` into the hand-written FortniteGame source,
include the header in module startup, and call `RegisterQueueConnectedFlowDriver()`.
Run the client with `-QueueConnectedFlow` and inspect `QUEUEFLOW v=1` rows. The
process requests exit status 0 only after all observations and teardown succeed;
a failed observation or the 180-second deadline requests status 1. Run with a
normal game world, local player and a ticking Slate viewport.

The probe uses a concrete activatable widget with ordinary SBox content. The
production pool obtains it, the production initializer precedes registration,
and a real queue attaches to the viewport. The flow waits across frames, removes
the first active widget, observes the second activation and deferred Slate
release, clears the queue, verifies pool reuse, and clears again. The displayed
widget callback captures the transient deferred array before the ticker clears
it. It does not exercise Blueprint widget content, every transition type,
multiple players, or arbitrary reentrant mutations. It requires the integrated
queue acquisition/removal and animated-switcher runtime implementations. No
build or runtime validation is performed by this component itself.
