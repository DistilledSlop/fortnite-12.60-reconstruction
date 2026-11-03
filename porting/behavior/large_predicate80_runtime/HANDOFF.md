# 0x141a5f1c0 rough recovery

One uncounted connected source draft, based on the pinned generated C in
`sources.json`. The ordered tables encode the repeated inline array scans and
later separate helper calls; each entry still obtains its own source and runs
its own predicate. The +0xa80 branch performs the conditional +0x638 scan.
The +0x358 and +0x360 return expressions differ for non-boolean `param2`.
The final +0x448 helper results are represented as their observed AL values;
Ghidra names them extraout_AL and extraout_AL_00, so the helper ABI needs
confirmation. The low source word is reloaded per inline element while the
high word is captured after the predicate. Array counts and pointers are read
once per array scan, and second-array reads occur only if needed.

Calls, object layout, array storage and native ABI remain unresolved; the draft
is not promoted or verified. No tests, compilation, Ghidra, generation, native
integration, staging or commits ran. Source inspection and `sha256sum` pinned
`porting/generated/callee_probe/blocking/0x141a5f1c0.c` to the hash in the
inventory. Root owns review and integration. Prior green baseline and product
gates are unchanged.
