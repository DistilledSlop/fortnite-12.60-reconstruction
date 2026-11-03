# UFortItemDefinition gameplay-tag interface

The original class registration at `0x146416900` identifies a native GameplayTagAssetInterface at object offset `0x30`. GPSC `0x14295c2b0` selects constructor `0x1420241d0`, whose store at `0x142024214` installs secondary table `0x14606c390`. Its `+0x10` entry, `0x1420302f0`, adds `0xb8` to the interface pointer and tail-calls the gameplay-tag-container copy routine `0x143e8c1b0`. Thus the source is object offset `0xe8`, the reflected `GameplayTags` struct. Assignment copies both the explicit and cached parent tags, replacing the output container.

The next three table entries (`0x143e915a0`, `0x143e90ec0`, `0x143e912e0`) call the same `+0x10` virtual, then test explicit and parent tag arrays for a single, all, or any requested tags. Empty all returns true and empty any returns false. These bodies match the behavior of the local engine's `GameplayTagAssetInterface.cpp` defaults, so the proposal relies on those defaults rather than declaring overrides. The engine interface header defines the matching virtual signatures. The reflected class has no `GetOwnedGameplayTags` function entry.

The original registration also lists FortTooltipInterface at offset `0x38`; this proposal does not reconstruct it. The policy appends the tag interface without suppressing any separately reconstructed interface. Original Windows offsets document evidence and do not dictate Linux C++ layout.

Run `python3 porting/behavior/item_tag_interface_runtime/prepare.py` to check the original image digest, pinned complete native target ranges, class metadata, constructor, secondary table, reflection, and proposal. Generator integration must invoke `source_policy.apply` and merge `manifest-entries.json`. This gate does not build or exercise the runtime.
