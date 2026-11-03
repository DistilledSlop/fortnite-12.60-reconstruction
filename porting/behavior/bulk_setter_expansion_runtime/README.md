# Object pointer setter recovery

`python3 porting/behavior/bulk_setter_expansion_runtime/prepare.py` scans the pinned original runtime image and reflection inventory. It emits source proposals here and evidence at `/tmp/gameplay-bulk-setter-expansion/{manifest-entries,report}.json`. The image SHA-256 is checked before decoding. This is source recovery only; the proposals have not been built or runtime tested.

The scanner requires a complete exact one-object-argument FFrame extraction thunk with verified branch targets and stack/register consistency, followed by a complete pure `mov [rcx+offset], rdx; ret` writer. It requires unique reflected class and field, matching object pointer type references, generated C++ pointer field declaration and accessible inheritance, and rejects replicated, static, return, reference and unsupported function/property flags. The report records the disassembly, writer, matched field and rejections. All other code shapes fail closed. The generated field assignment preserves the original direct store; no callback, validation, or additional effect is inferred.

On the current inventory this yields 18 proposals, pending integration and the next scheduled validation batch. Existing canonical manifest and generated Native files are not modified by this scanner.
