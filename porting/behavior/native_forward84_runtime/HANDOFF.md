# Native forwarding probe 84

No proposals: no meaningful zero-argument exec tail jump matched an already
accepted `helper`, `callee`, `static_helper`, `impl`, or `native_event_impl`
address. The sole raw match was `UPrimitiveStatsProfilingHelper::EnablePrimitiveStatsCapture`
exec `0x142b8a060` to `0x1406801c0`, a shared empty implementation stub;
it is outside the requested meaningful behavior and its frame sequence also
clobbers `rcx`. No manifest entries or implementation bodies were authored.

Inventory command: `python3 /tmp/forward84_scan.py | head -80` (exit 0).
The scratch script used `native_funcs.json`, `native_class_set.txt`, canonical
`MANIFEST.json`, Capstone from `/tmp/common-input-capstone`, and mmap of
`/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`
(the image previously pinned by `bulk_leaf_mutation83_runtime/scan.py` as
SHA-256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`;
this probe did not recompute the checksum). It inspected no-argument entries
not marked ported, scanning the first 80 bytes until the first jump or return;
the final direct jump target was compared to the exact `address` field of
accepted helpers and implementations. It did not filter function flags or
prove eligible declarations, because no meaningful target survived. It does
not cover call/return wrappers, indirect or conditional control flow, or
helpers whose accepted address is only present in notes.

Next avenue: inspect no-argument execs that call an accepted typed helper then
return or transfer a result, preserving Win64 argument setup and return ABI;
filter out ProcessEvent RPC wrappers and prove class access/dispatch and flags
before proposal. Separately index accepted helper addresses in notes only after
verifying each mapping against its supporting evidence. Tests/build/generation
were not run; last tested baseline 2,040 bodies, next checkpoint about 3,040.
