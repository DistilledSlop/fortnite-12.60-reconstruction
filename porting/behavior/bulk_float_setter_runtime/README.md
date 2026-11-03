# Float setter recovery

`prepare.py` extends `bulk_parameter_setter_runtime` (int32) to one-`float` setters. It reuses the pinned original image (sha256 checked), the Step/StepExplicitProperty branch-target and P_FINISH guards, and the class/flag/header guards. Parameter and field records go through `gen_native_classes.split_properties`, so enum/array/map/set children are excluded. Fields resolve by offset along the proven parent chain. An inherited field must be non-private in its owner header. Output goes to `/tmp/gameplay-bulk-float-setter/{manifest-entries,report}.json` plus the `.inc` files here. Nothing canonical is modified and nothing was built or run.

Result: 13 candidates (10 inline `movss [rdi+off], xmm0`, 3 direct calls to a `movss [rcx+off], xmm1; ret` leaf) out of 1200 float-kind parameter records.

Reusable blockers (counts are sequential rejections):
- 225 direct-call callees are not a single pure store. 48 are empty `ret` stubs (no-op; not emitted). 35 are undecodable within 256 bytes. The rest have change detection (`ucomiss` plus dirty byte), `movaps xmm6` spills, a global flag check, or a call.
- 17 inline setters also write a dirty byte (e.g. `UFortClientSettingsRecord::SetGamepadBuildingMultiplier`); this needs the flag field mapped to be complete.
- 12 thunks dispatch through a vtable (`call [rax+N]`).
- 11 offsets have a missing or ambiguous field or a non-float type.
- Generated headers declare no member functions, so signatures are `proposed` from the reflection parameter only.
- The remaining float records are return/out/ref parameters (`GetX`) or multi-parameter functions, which are out of scope.
- Bool, object and name setters are not attempted.
