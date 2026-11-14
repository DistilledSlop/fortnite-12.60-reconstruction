# Movement attribute constructor projection

In original-late-capture-1/image-125.runtime.bin, the UFortMovementSet GPSC 0x1429f5ec0 passes 0x1429f67d0 at 0x1429f5f25 as its native constructor thunk; that thunk loads Initializer.Obj and jumps to the actual instance constructor 0x141b8bd50. Exact instruction captures are in this directory. The parent constructor is called at 0x141b8bd72, and the body returns at 0x141b8c01f. It installs the class vtable and each attribute's vtable and writes eleven 0x28-byte attribute structs beginning at +0x30. Their base/current and unclamped fields are zero, bounds bit patterns ff7fffff/7f7fffff, and three flags false, matching the recovered struct-ops constructor 0x142840060. The final four other-than-default setter pairs use the float at 0x1459a86f8 (bits 3f800000, 1.0):

| Reflected field | Original offset | Base call | Current call |
| --- | --- | --- | --- |
| BackwardSpeedMultiplier | 0x120 | 0x141b8bfa4 | 0x141b8bfb2 |
| GravityZScale | 0x170 | 0x141b8bfbe | 0x141b8bfca |
| VehicleGravityZScale | 0x198 | 0x141b8bfd6 | 0x141b8bfe2 |
| SpeedMultiplier | 0x1c0 | 0x141b8bfee | 0x141b8bffa |

Vtable 0x145ef12c0 entries +0x10 and +8 are 0x141bc47e0 (base setter) and 0x141bc4960 (current setter); the first base call is devirtualized. Both set value and unclamped value to 1, leaving bounds and flags as above. WalkSpeed, RunSpeed, SprintSpeed, FlySpeed, CrouchedRunSpeed, CrouchedSprintSpeed, and JumpHeight retain zero base/current. Names and offsets map to original reflection block 0x146468bc0, each struct reference 0x142841da0. No claim is made about later asset, CDO, gameplay effect, replication, or runtime values.

The production generator requires the complete recovered fort_attribute_data_runtime policy, including its exact constructor and setters, active and validated before class generation. Invoke validate_struct for all struct models on its declaration output, and finalize for classes after other constructor-producing policies; fail on conflicting member uses. The class guard pins parent, eleven names, types, and original reflection offsets. Existing constructor side effects are preserved but its use of any selected field is rejected. The projection reproduces reflected fields only: original parent behavior, vtable installation, object/struct lifecycle, padding and any unreflected side effects are not implemented or independently claimed. Default-subobject/archetype initialization and later serialization must retain normal ordering. The root integration activates these hooks. Build and live acceptance are recorded on the canonical wiki.

Capture SHA-256: `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`. The test uses the actual reflection class block and pre-integration source fixtures from commit `251b7c85`, exercises constructor creation/composition, rejects field, offset and constructor conflicts, and checks the struct declaration dependency. Run `porting/generated/.venv/bin/python porting/generator_policies/movement_attribute_defaults_runtime/test_policy.py`.
