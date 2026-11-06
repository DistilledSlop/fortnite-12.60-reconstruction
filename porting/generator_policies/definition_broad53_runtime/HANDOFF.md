# Definition queries batch 53

13 complete reflected query bodies across six definition classes. `manifest-entries.json` contains proposals; `.inc` files contain bodies. Native wrapper and callee bytes are pinned in `native-evidence.json`. Signatures use reflected return types and const flags; field names/types are from generated class declarations. No explicit generated-class headers are proposed.

`GetObjectiveNeededToComplete` returns the configured positive completion count or the objectives array count. `GetReticleCornerAngles` returns a const array reference: reflection marks the return const/reference and its wrapper copies the callee reference into the result. `GetTriggerType` shares an existing native helper callee but is a distinct reflected function. `GetUIMissionPointsOffset` clamps negative values to zero.

No canonical manifest or generated files were edited; no build or tests were run. Integrator should check signatures and splice/build under its usual workflow.

The signature policy restores only the native-proven const-reference array return, checking class identity and reflected CPF_ConstParm, CPF_ReturnParm and CPF_ReferenceParm before the splice gate. The generic decoder previously discarded return-reference qualifiers.
