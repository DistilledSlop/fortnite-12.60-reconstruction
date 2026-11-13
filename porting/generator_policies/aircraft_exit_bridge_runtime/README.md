# Aircraft exit compatibility bridge

The generated game mode exposes `OnAircraftExitedDropZone` as a UFUNCTION and
forwards it to the existing match-flow callback. The aircraft authority helper
can then fire all three delegates synchronously, preserving exit-before-ended
ordering and the existing forced-exit and safe-zone behavior. No original exit
callback body is claimed recovered by this bridge.

The guarded reconstruction fixture contains a native function entry and two
parameter candidates agreeing on one aircraft pointer. Its class name and
candidate owners are unresolved; the fixture alone does not prove original
owner identity. The class association follows the existing match-flow
reconstruction and the existing flight-ended unbind of this callback name.

The policy rejects a future competing model/declaration/definition so complete
original callback recovery must explicitly replace this bridge.
