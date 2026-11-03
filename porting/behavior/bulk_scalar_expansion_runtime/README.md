# Read-only float literal wrappers

`prepare.py` scans the selected reflected native functions against the pinned
runtime image SHA-256. It accepts only a zero-argument `Float` return with the
usual complete frame-advancing wrapper, a direct call, and a complete leaf of
exactly `movss xmm0, [rip+disp32]; ret`. The literal must fall entirely inside
the pinned image's non-writable `.rdata` section. It rejects nonfinite values
and negative zero; nine significant digits preserve the accepted binary32
values in the generated C++ literals. Reflection flags, access, class,
parameter shape and existing ports are checked before emitting a proposal.

Run `python3 porting/behavior/bulk_scalar_expansion_runtime/prepare.py` to
write local `.inc` files and `/tmp/gameplay-bulk-scalar-expansion/{manifest-entries,report}.json`.
`--revalidate-own` rescans this directory's entries after promotion. The report
records wrapper and leaf disassembly, source literal address and binary32 bits.
It does not modify the canonical manifest or generated Native. These are source
reconstructions; they have not been built or runtime tested.
