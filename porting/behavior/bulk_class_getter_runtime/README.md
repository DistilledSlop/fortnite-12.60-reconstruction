# Class getter proposals

`python3 porting/behavior/bulk_class_getter_runtime/prepare.py` scans the SHA-pinned original image and writes proposals and complete instruction evidence to `/tmp/gameplay-bulk-class-getter/`. It accepts complete inline or wrapper-plus-leaf qword copies only, matching the same-class reflected Class field and return's two exact type references and the generated `TSubclassOf<ExactType>` field and return. It excludes existing ported, maintained and non-stub generated definitions, incompatible declarations and unsupported flags.

In UE 4.25 `Templates/SubclassOf.h`, the compiler-generated same-type copy constructor copies the sole `Class` member; the templated cross-type constructor dereferences and checks, as do `Get` and conversion to `UClass*`. The identical generated types make `return Field;` use the former. This is source recovery, not build or runtime validation.

Integrated in native source batch 17: one body. Compilation and runtime checks remain pending at approximately 2,040 bodies.
