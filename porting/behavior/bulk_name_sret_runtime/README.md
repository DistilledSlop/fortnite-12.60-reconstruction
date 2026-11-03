# FName sret getter proposals

`prepare.py` finds zero-argument FName getters whose original code is a complete Win64 sret wrapper: `push rbx; sub rsp,0x20`, the exact script-frame advance, `mov rbx,r8`, `lea rdx,[rsp+0x38]` (8-byte temp), one call, an 8-byte copy of `[rax]` to the output, epilogue and `ret`. The called leaf must be a complete raw copy of `this+offset` to `RDX` (one qword, or two adjacent dwords) returning `RAX=RDX`, with no other effects.

The offset must match exactly one own, top-level (`split_properties`, no children) reflected `Name` field of the same selected, unique FortniteGame/FortniteUI class, resolved by the generator to `FName` and declared once in the current generated header. Inherited/unreflected fields, constants, globals, constructors and dynamic lookups are rejected, as are already-ported functions. The `.inc` body is `return Field;`.

Writes guarded `.inc` proposals here and a report plus proposed manifest entries to `/tmp/gameplay-bulk-name-sret/` (`--output-dir` to change; `--revalidate-own` rechecks this directory's entries). It does not touch the canonical manifest or generated code.
