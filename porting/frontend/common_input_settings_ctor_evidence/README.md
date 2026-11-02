# CommonInput settings constructor

Bounded source recovery from the original captured runtime image, not a runtime
parity claim. `PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_settings_ctor_evidence/replay.py` decodes the listed bounded windows. Existing reflection identifies settings size 0x1b8 and the platform struct members; the class getter `14081df50` registers constructor thunk `14081e220`, which takes the object pointer from the initializer, then forwards to `14081a400`. That body calls the UObject base constructor and installs the settings vtable before initializing fields.

The constructor initializes InputData's soft pointer at +0x28 with weak index -1, serial 0, tag 0 and an empty path. It sets thrashing protection +0x168 true, limit +0x16c 30, window +0x170 3.0 and cooldown +0x178 1.0. It explicitly sets the unreflected cache-valid byte +0x180 to false and InputDataClass +0x188 to null. CurrentPlatform at +0x190 starts with scalar bytes `00 01 00 01 00 01 00` and empty soft/class arrays. The seven array elements at +0x50 have empty arrays and the following seven scalar bytes, in member declaration order (supported, default input, mouse/keyboard, gamepad, default gamepad, change gamepad, touch):

| Index | Offset | Initial scalar bytes |
|---|---|---|
| 0 | 0x50 | `01 00 01 01 00 01 00` |
| 1 | 0x78 | `01 00 01 01 00 01 00` |
| 2 | 0xa0 | `00 01 01 01 01 00 00` |
| 3 | 0xc8 | `00 01 00 01 00 00 00` |
| 4 | 0xf0 | `00 02 00 01 00 01 01` |
| 5 | 0x118 | `00 02 00 01 00 01 01` |
| 6 | 0x140 | `00 01 01 01 02 00 01` |

These values are the final result of the individual and overlapping stores in
`14081a400..14081a692`. Some first fields are changed late in the constructor.
All seven slots are initialized unconditionally; there is no platform choice in
this body. The later cache initializer copies element 0 at +0x50 to
CurrentPlatform, as described in `../common_input_platform_evidence/README.md`.
Constructor defaults do not establish the values after Unreal applies config or
copies template properties; retain the ordinary UObject/config lifecycle.

The module startup's outer getter `142e80820` returns a single global pointer.
The shape matches UE's GetTransientPackage but no independent identity proof was
found here; the startup evidence in `../common_input_module_evidence/HANDOFF.md`
remains qualified. No module registration linkage, generated/native code or
runtime behavior was changed.
