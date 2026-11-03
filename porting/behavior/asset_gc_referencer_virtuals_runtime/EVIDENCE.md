# FFortAssets referencer virtuals

Captured image: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, mapped base `0x140000000`. SHA-256 below is over the exact contiguous function bytes in that image (offset = address minus base).

| Function | Address | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| GetReferencerName | `0x141dfad80` | 81 | `7c65908217f042ea2ae4a6df36f2861ca98d681fd2e4ca38a451bd5d343c4c80` |
| GetReferencerPropertyName | `0x141dfade0` | 762 | `9fe1b5f0df6cb88a22290960a02368c9a63a1e5e7e0ae1fa53284aec0284f14f` |

Ghidra probe output: `porting/generated/asset_gc_referencer_virtuals/`, generated with the adjacent `targets.json`. First function copies UTF-16 literal `FFortAssets` at `0x145fdbe10`. The second tests null before locking owner `+0x1a0`, tests sets `+0xb0` then `+0x100`, and assigns literals `AlwaysLoadedAssets` and `LoadedAssets`. It releases the lock before map `+0x150` lookup; absent entry or a zero FName returns false without changing the output; otherwise it assigns `GCPreventionPool.` concatenated with the FName string. The source fragment maps those offsets to the owner reconstruction's field names. It uses const_cast solely to acquire the existing critical section from this const virtual; declaring that member mutable is an equivalent integration choice. No tests or build were run per scope.
