# Ignored enum parameter recovery

`python3 porting/behavior/bulk_ignored_enum_runtime/prepare.py` scans the SHA-256 pinned original runtime for ordinary void native functions with exactly one by-value byte-backed Enum or enum-typed Byte parameter. Reflection records are split with the generator; Enum requires its explicit trailing Byte underlying record. The enum reference resolves through generator TypeTables, its package must match the unique generated enum header, and the generated declaration and `decode_params` spelling are checked. Enum properties require a generated `enum class : uint8`; Byte properties prove byte storage from their reflected property kind. Existing generated function declarations, if present, must match; functions not yet promoted need not be present in the header.

The complete FFrame wrapper, extraction stack slots and registers, both branch destinations and the direct leaf are matched; only a complete `ret` or `ret 0` leaf qualifies. Selected classes, class/package metadata, flags and existing canonical or maintained bodies are excluded as in bulk_ignored_extra_runtime. Evidence includes metadata, generated enum identity, complete disassembly and leaf. Unsupported widths and unresolved types are rejected.

The scan returned seven proposals (six Enum, one Byte) from 23,758 records. Artifacts: `/tmp/gameplay-bulk-ignored-enum/{report,manifest-entries}.json`. These are source proposals, not runtime validation. No canonical manifest or Native source changed. No build, test or runtime run; the next integration checkpoint remains about 2,040 bodies.

Integrated in native source batch 15: 7 bodies; compilation and runtime checks remain pending at approximately 2,040 bodies.
