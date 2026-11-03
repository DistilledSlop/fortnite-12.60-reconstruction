# Reflected full-byte bool getters

`prepare.py` scans the pinned original runtime image and reflection for complete
zero-argument Boolean thunks that load a byte directly, either inline or via an
exact pure leaf. It accepts only a unique matching reflected Boolean with
`bool_bits` form `mov8`, mask 1, and a generated `bool` declaration. It checks
class identity, inheritance, function flags, field accessibility, and existing
manifest and generated definitions. It emits isolated source proposals and a
scratch manifest/report, without modifying canonical Native or the manifest.
The original byte read assumes a valid Boolean representation, as does the
reflected C++ `bool`; arbitrary noncanonical byte values are not modeled.

Run `python3 porting/behavior/bulk_bool_byte_runtime/prepare.py`; use
`--revalidate-own` after promotion to rescan this directory's entries. The
image SHA-256 and complete disassembly matches are recorded in the report at
`/tmp/gameplay-bulk-bool-byte/report.json`. These are source recoveries; build
and runtime validation are pending the project's integration cadence.
