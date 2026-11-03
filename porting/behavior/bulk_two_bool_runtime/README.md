# Two-input reflected Boolean leaves

`python3 -I porting/behavior/bulk_two_bool_runtime/prepare.py` examines two finite candidates against the SHA-pinned original image. It checks both complete FFrame normalization wrappers and branch destinations, complete direct leaves, generator signatures, and reflected Boolean identities and declarations. Parameter table offsets are absent, so the scan reads each original parameter property and its SetBitFunc to establish offsets 0 and 1, then cross-checks the generator's argument ordering. Reflected fields must have `bool_bits.json` `mov8`/mask 1, no other reflected property at the offset, and accessible generated `bool` declarations. The scanner makes no canonical or Native changes.

The two source-only proposals and disassembly are written to `/tmp/gameplay-bulk-two-bool/{manifest-entries,report}.json`. `SetBuildingAllowed` assigns each normalized argument; `DirtyBotFactions` ORs each normalized argument into the corresponding existing field. As in `bulk_dirty_scalar_runtime`, this assumes valid C++ Boolean representations; noncanonical underlying bytes are not modeled. The proposal does not claim build or runtime validation.

Integrated in native source batch 19: two bodies. Compilation/runtime checks remain pending at approximately 2,040 bodies.
