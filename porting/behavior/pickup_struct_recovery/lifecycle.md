# Property override native lifecycle

The original registration functions `0x142aabb90` and `0x142aabc20` return reflection descriptors; they are not constructors. The `TCppStructOps` vtables recovered from `struct_blocks.json` are `0x1464f0990` (FPropertyOverrideData) and `0x1464f0ac0` (FPropertyOverrideMk2). Slots 3, 5 and 21 correspond to Construct, Destruct and Copy in the target engine's `ICppStructOps`:

| Struct | Construct | Destruct | Copy |
| --- | --- | --- | --- |
| FPropertyOverrideData | `0x142aaa100` | `0x142aaa720` | `0x142aaa400` |
| FPropertyOverrideMk2 | `0x142aaa1a0` | `0x14077cb40` | `0x142aaa5e0` |

The first constructor initializes the native-only word at +0x30 to two 32-bit values `0xffffffff, 0`; the second does the same at +0x40. Both constructors zero their reflected arrays or strings. Data additionally sets its override mode byte at +0x38 to 1 and its reflected object pointers at +0x40 and +0x48 to null. Each copy operation copies the corresponding 8 bytes directly. Mk2 destruction releases only its four FString allocations. Data destruction releases its two ID array allocations and calls `0x1419b9a20`, which destroys the four strings in each override element and releases the override array. Neither destructor touches the native-only word.

The initialized pair exactly matches the target engine `FWeakObjectPtr::Reset()` (`ObjectIndex = INDEX_NONE`, `ObjectSerialNumber = 0`), and its default copy/destruction behavior is compatible. This strongly suggests weak-object storage but does not establish the semantic referent or prove the type: neither constructor, copy, nor destructor identifies what consumes the word. Declaring `FWeakObjectPtr` or a typed `TWeakObjectPtr` as an established semantic field would therefore overstate the evidence.

For layout and the observed lifecycle alone, generator support can use two native, non-reflected 32-bit fields initialized to `-1` and `0`, preserving their offsets, default construction, independent memberwise copying and trivial destruction. An 8-byte scalar with the equivalent low/high bit pattern also matches these operations on the target little-endian ABI, but explicit halves expose the observed initialization. This does not establish compatibility with native operations that interpret or mutate the value. Generator integration must retain exact offsets, sizes, alignment and constructor defaults, and verify resulting ABI; usages and object semantics remain a blocker for claiming full parity. Existing reflected `OverrideMode` default also needs preservation when constructing Data.

Evidence is from the local original runtime image and Ghidra decompilation of the listed addresses, with vtable slots cross-checked against the local UE `CoreUObject/Public/UObject/Class.h`. No runtime validation was performed.
