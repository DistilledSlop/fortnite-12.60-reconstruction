# Diagnostic asset key readers

Source batch41 replaces the earlier explicit borrowed-map binding with the actual
shared FFortAssets owner, as in the original. Both static UFortAssetManager helpers
take only the borrowed output array; their former map parameters are removed.
Singleton calls remain at the original count/iteration points. The owner friends
UFortAssetManager solely to read its actual maps; there is no parallel index.
Map hash/mutation writers remain open, and no hashing/mutation is introduced.
The SAME FFortAssetDiagnosticMap alias is used by owner and helpers.

At 0x141df6860, the original computes the sum of the two live entry counts, compares it to destination capacity, and reserves that count only if greater. It then appends keys in sparse map iteration order from +0x10 followed by +0x60. At 0x141dfb390 the equivalent count and traversal use +0x10 only. The capacity comparison is against map count, not existing destination length plus count: subsequent `Add` operations may grow the array. Existing contents remain, and duplicate keys across maps remain duplicated. Each append copies the complete path, including the deep-copied FString; the int32 values are not copied. No sorting or deduplication is justified.

Raw image windows and SHA-256 pins are in `native-evidence.json`. At 0x141df688c–0x141df68aa and 0x141dfb3bd–0x141dfb3cb the count/capacity comparisons precede the reserve call; 0x141df6a5b–0x141df6a98, 0x141df6be8–0x141df6c25, and 0x141dfb568–0x141dfb5a5 show element stride 0x28, output stride 0x18, and path string copy via 0x140682ce0. The second map is accessed at static address 0x14812bc00 in the first reader after the first traversal; its location corresponds to the owner's +0x60. See the prior handoff for the type evidence and limitations.

The support header must be placed on the eventual include path and the proposed manifest entries integrated with their declarations; no canonical files were edited. No Ghidra session was used, and no build, tests, generated output, or commit was made.
