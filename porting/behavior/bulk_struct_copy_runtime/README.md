# Bulk POD-struct field-copy getter proposals

`python3 porting/behavior/bulk_struct_copy_runtime/prepare.py` scans the pinned original
runtime image (SHA-256 checked) and reflected native inventory once. It emits individual
`return Field;` bodies here, plus a proposal manifest and detailed evidence (matched
instructions, leaf, reflected offset) under `/tmp/gameplay-bulk-struct-copy/`.
`--output-dir PATH` changes the report destination; `--revalidate-own` includes entries
already ported from this directory. Existing bodies must match exactly or the scan stops.

Scope: zero-argument, non-static, non-RPC, non-BlueprintEvent, non-editor getters returning
one of the CoreUObject POD structs FVector(12), FRotator(12), FVector2D(8), FLinearColor(16),
FGuid(16), FColor(4), FQuat(16), FVector4(16). `generator.TypeTables` must resolve the
return's struct reference to that exact name, package and size. The return property flags
must be exactly Parm|OutParm|ReturnParm (no const/reference/array). The same selected,
unique, non-interface class must reflect a top-level (via `split_properties`) field with
the identical struct reference at the copied offset; `Resolver` must spell return and
field identically and the generated header must declare that field. Inherited, native
unreflected and padding-guessed fields are rejected.

Two code shapes qualify, both requiring the full P_FINISH frame advance:
* inline: byte-copy from `[rcx+field]` to `[r8]`;
* wrapper: `push rbx; sub rsp,S` (16-aligned), temp sret buffer `[rsp+T]` inside the
  allocated area above the shadow space, direct call to a leaf that copies
  `[rcx+field]` -> `[rdx]` with `rax=rdx`, wrapper copy `[rax]` -> `[rbx]`, and matching
  `add rsp,S; pop rbx; ret`.
The byte-coverage checker only accepts mov/movss/movsd/movups/movdqu of proven registers:
every load comes from the source base inside the field, every store comes from a loaded
value at one constant delta, stores cover exactly `[0,N)`, and no other instruction exists.
Arithmetic, shuffles, aligned moves, virtual calls, conditionals and other calls are rejected.
Proposals are source recovery only; they require integration and validation.
