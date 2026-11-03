# Constant Boolean returns with an ignored argument

`python3 -I porting/behavior/bulk_parameter_constant_runtime/prepare.py` scans the
SHA-256-pinned runtime image and writes proposals and complete instruction evidence
under `/tmp/gameplay-bulk-parameter-constant/`. It does not change the canonical
manifest or Native source.

Three literal complete wrapper forms cover instance object arguments, instance
int32/uint32 arguments, and static object arguments. Both FFrame extraction paths,
branch destinations, stack slots, argument registers, frame advancement, saved
register restoration, and byte result storage must match exactly. The entire
direct leaf must be `xor al, al; ret` or `mov al, 1; ret`. The source body returns
that exact Boolean and intentionally ignores the input. Strings, structs,
references, events, RPCs, other wrappers and side effects remain excluded.

Reflection and generator parameter decoding establish the complete signature;
selected classes, class/package identity, resolved selected object types,
const/static/access flags, and existing declarations are checked. Canonical bodies,
maintained definitions and generated inline or out-of-line definitions are
excluded. Six proposals were recovered. Compilation and runtime checks remain
pending at the shared approximately 2,040-body checkpoint. Source coverage alone
does not establish feature parity.

Integrated in native source batch 15: 6 bodies; compilation and runtime checks remain pending at approximately 2,040 bodies.
