# CommonButton production flow diagnostic

Opt-in `-CommonButtonProductionFlowProbe`; hook `RegisterCommonButtonProductionFlowProbeDriver` in client startup and compile the two source files in FortniteGame after the production Initialize, SetStyle and receiver graph is promoted. `verify.py LOG` rejects any missing, duplicate, failed, reordered, malformed or extra BUTTONFLOW observation.

The controlled game-world player context owns two manually created button widgets with real WidgetTrees. The test invokes the production initializer, including its original null-root weak-only branch and a root-containing branch. It checks parent initialization timing, one-shot behavior, GC, copied fields, slot geometry and all five bindings. Real delegate broadcasts exercise focus, selection, click rejection/toggle, double click and touch press/release; style tests use the public setter including a reflected wrong-class argument and synchronous reentry. Style CDO dimensions and input type are restored.

This diagnostic is unbuilt until the integrating client build, and does not establish actual hardware routing, authentic frontend, pixels, audio, NativeCommit raw-byte behavior, or null-root callbacks. It deliberately does not assign the internal weak pointer. An early failure produces a truncated transcript that fails verification.
