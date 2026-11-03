# Pawn actions 73 runtime handoff

Six distinct actual native bodies were lifted from the serial probe, not six generated exec wrappers. `rough.cpp` is a connected, deliberately unresolved behavioral lift; it is not an ABI-compatible implementation or a build-ready replacement. `targets.json` is copied from the probe. No tests, compiler, Ghidra, regeneration, or commits were run.

The external adapters name unresolved calls, global values, virtual effects, and ABI boundaries. Their implementations must preserve the original calls and observable side effects, including callback-induced changes to memory. In particular:

- `spherical_math_142c521d0` receives the original incoming-direction argument alongside the decompiled matrix and scalar. The apparent call does not account for that argument or establish the output ABI; do not infer a direction formula from the decompiler signature.
- Missing-socket logging is a single explicit contract covering allocation, four string conversions including the omitted-argument call, the unresolved log varargs, and cleanup order. Its implementation must reproduce intermediate reads and effects, including mesh/name rereads if callbacks mutate them. The generated C does not recover enough arguments to wire these calls safely.
- Feedback payload bits 24 through 63 come from five uninitialized prior-stack bytes, modeled as an explicit adapter; they are not zero-initialized. The lower 24 bits are `0x050000`.
- Knockback preserves the two rsqrt refinements, comparison branches (including unordered/NaN behavior), and reads the movement flags again after the first virtual callback. Adapter for the small-vector fallback must supply the actual three global float values. Exact floating point behavior requires SSE scalar operations and compiler settings that prevent contraction or reassociation; the C++ is a rough lift, not a guarantee of those settings.
- Pointer and type metadata validity, names, virtual call ABIs, and global constants remain unresolved. The engine's actual allocation/diagnostic behavior must be supplied by adapters; none may silently do nothing or invent defaults.

Source SHA-256 inventory (`porting/generated/pawn-broad73/`):

```text
60dd5e905446c6c1393ec7f0d65020b1729cd3ebfe089d497dba49cd96513230  index.json
782ef34054c8a7d4971351872441bb0f8a46faf99310b21652680abf7f9d438d  targets.json
6ad7361b36b056cfb381f5e047e646f9f953f0598c925d153d349f4a76848ae5  0x142304420.c
383ccb16e78347ff39d48563e83701370d22dc2d898b4e164a806542b812fec2  0x1423055b0.c
d3c866b327cd8e6e2f225827dd7104aa31338e619865112a15227af88051fd5d  0x142305f70.c
cdb403a485398caa94bf20578d69b783a124a8076e5aac981f0121ad18c9859e  0x1423249d0.c
f1a27fa43af7cf63796f3158c728b0bcb25c1b096015933df374a11bd1891677  0x14232f830.c
2f093e976af31833358fe31ed13e0eea1017942a71e92934992c20c216ad35ee  0x142356fc0.c
```

Root probe: `porting/.venv/bin/python -I porting/ghidra/probe_callees.py --input porting/generated/pawn-broad73/targets.json --out porting/generated/pawn-broad73 --timeout 60 --mem 12G > /tmp/pawn-broad73.log 2>&1` exited0; six statuses ok, elapsed11.1s. The transaction rolled back; usual Ghidra project index/prp metadata changes were reported, with no deliberate project save. No build/compiler/runtime/tests. Root reviewed all six bodies against probe output and renamed the opaque 12-byte socket parameters Raw12, without asserting name/vector semantics.
