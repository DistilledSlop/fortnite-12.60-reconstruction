# Native class size evidence

`python3 porting/behavior/native_class_size_evidence_runtime/scan.py` writes
`/tmp/gameplay-native-class-size/report.json`. It requires the pinned runtime
image and the existing Capstone installation at `/tmp/common-input-capstone`.
The image SHA-256 is checked before any extraction. This is evidence generation
only; it does not change or authorize generated native code.

The scanner accepts one complete, straight-line MSVC Win64 registration family:
`mov r11,rsp; sub rsp,0x88`, an exact null-check and register/stack setup,
the immediate dword store at `[rsp+0x20]`, a direct call to
`0x142d698b0`, and the matching return sequence. All instructions in the
family are matched, including the branch destination and matching static-class
storage references, so no unexamined stack adjustment, intervening write, or
alternate call can alter the argument. Under the Win64 call convention the
fifth argument is at `[rsp+0x20]` immediately before `call`. The adjacent
engine `CoreUObject/Public/UObject/Class.h` declaration names argument five
`uint32 InSize`; `ObjectMacros.h` `IMPLEMENT_CLASS` passes `sizeof(TClass)`
there. The registration target identification is documented in
`porting/decompilation_notes.md`.

A record additionally requires a unique indexed GPSC target, matching reflection
block/class identity, and exact UTF-16 package and unprefixed class literals in
the call setup. Each record includes the complete decoded instruction sequence,
call and store addresses, class block and any reflected Bool sizes. A class
with a different instruction family is omitted, not inferred. Multiple Bool
sizes or a disagreement with a proven immediate aborts report generation. The
scanner does not establish original field offsets, safe layout, class hierarchy,
or parity of a reconstructed class; consumers must establish those separately.
