# Additional ignored-parameter recovery

`python3 porting/behavior/bulk_ignored_extra_runtime/prepare.py` scans ordinary native void functions with one by-value `Name`, untyped `Byte`, or `UInt32` input and no other properties. It uses the exact direct FFrame wrapper recognizer from the single-parameter int/object scanner for Name/UInt32, and an exact byte-specific variation of the float direct wrapper. It verifies extraction locals, register widths, saved stack slots, both conditional branch destinations, frame advancement, and a direct leaf consisting entirely of `ret` or `ret 0`. Input kinds and declarations match the native generator's by-value mappings. The original image is SHA-256 pinned. Class/flags/selection, metadata, existing maintained/ported definitions and header declarations are guarded as in the prior ignored-parameter scanner.

The generated comment-only bodies are proposals for complete empty implementations; arguments are ignored. Run produced 18 proposals: 14 FName, 2 uint8, 2 uint32, all calling ret-only leaf `0x1406801c0`. It examined 23,758 reflection records; evidence and proposed manifest entries are in `/tmp/gameplay-bulk-ignored-extra/{report,manifest-entries}.json`. No canonical manifest or native sources were changed. No build, runtime, or tests were run; source proposals await integration and validation.

Enum-typed Byte and Enum inputs are deliberately excluded: type resolution, enum underlying storage and generated declarations require additional proof. Nontrivial extraction patterns and nonempty or virtual leaves are also excluded.

Integrated in native source batch 14: 18 bodies. Compilation and runtime validation remain pending at the approximately 2,040-body checkpoint. Source coverage only; no feature or milestone parity claim.
