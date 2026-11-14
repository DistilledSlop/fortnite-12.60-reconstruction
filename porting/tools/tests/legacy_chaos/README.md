# Legacy Chaos parser validation

Run from any working directory:

```sh
python3 porting/tools/tests/legacy_chaos/validate.py [--engine-root /path/to/UnrealEngine]
```

The default Engine checkout is the sibling `UnrealEngine` directory. The test compiles its installed `Engine/Source/Runtime/Engine/Private/PhysicsEngine/FortniteLegacyChaosCookedData.h` against the local minimal Unreal shim using `g++`, C++14, warnings as errors, and UBSan. Python assertions are required; do not use `python -O`.

The recovered Linux stage under `porting/generated/linux-stage/LinuxNoEditor/FortniteLinux/Content` must contain the sidewalk and oil rig `.uexp` fixtures. The existing independent inspector tools pin and decode those fixtures. Game bytes are not stored in this test. The suite compares decoded geometry, exercises synthetic and malformed inputs, and rejects every proper fixture prefix. The executable, temporary input, and JSON results are written to `porting/generated/agents/legacy-parser-retained-tests44/`.
