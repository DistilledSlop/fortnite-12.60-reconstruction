# Feedback and volume rough lifts

`rough.cpp` covers the full decompiled bodies of 0x1424c21f0 and 0x1419c3880. Feedback includes both configuration paths, null and cooldown exits, actor selection, probability and timing gates, payload construction, dispatch, and the five global-pair callback cases. Volume includes lookup and output assignment, the Athena class ancestry test and early return, component lookup, temporary construction, result, allocation release and two-stage reference-count destruction. There are no loops in either supplied body. Offsets, short-circuit gates, callbacks, and post-dispatch reads are retained.

This is a rough unlinked behavioral lift, not a promoted implementation. Neutral externs represent individual unresolved native functions, virtual dispatch ABI, the vtable address and global loads; they require integration with real ABI/types. `DispatchPayload` and `VolumeTemporary` reflect the decompiler's stack layout and require verification against native ABI and constructors. The global address accessor for 0x145a95888 must return the address of the vtable itself. The temporary's construction must populate all subsequently read fields. Native float/NaN, random, memory and reference-count behavior must be retained by integration. No compilation, tests, generation, or native promotion was performed.

Source inventory (SHA-256):

- `porting/generated/native-broad75/0x1424c21f0.c`: `45a728b4ec66ab2cadc1da7187542c8dadaf7520df909095a69e8d09a67204d5`
- `porting/generated/native-broad75/0x1424c21f0.json`: `a987f01c1864175478d46bd00a346524b67fcba2c93b4358251087da56c2a979`
- `porting/generated/native-broad75/0x1424c21f0.asm`: `70f975b24bd84aeb14a422753820defd19dc48e4473819e6761682b4323b64f8`
- `porting/generated/native-broad75/0x1419c3880.c`: `3fc5a36f2f2abf385426695ecbb1434df77b649d35f830125ea92af798260805`
- `porting/generated/native-broad75/0x1419c3880.json`: `563df4d0d17714b9313fd0bb6ade9ac2cc5dc487596a0735ae0880bd10f5b9a1`
- `porting/generated/native-broad75/0x1419c3880.asm`: `0cd980b32aff2ff458bbe88b50f2fb93d028011b0e47cdebe2cabd164ddeda96`

The `.c` files were the source of the lift; the `.asm` and `.json` hashes identify the corresponding source inventory and were not independently audited.
