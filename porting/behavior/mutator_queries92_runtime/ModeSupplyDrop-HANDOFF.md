# Mode supply-drop helpers

`ModeSupplyDrop.cpp` proposes native bodies 0x141949a30 (spawn range) and 0x141949b10 (speed range), based on `porting/generated/native93-loot/` decompilation and disassembly. The output pointers may alias: first value is stored before the second row-value call and second store. Each row value is signed int32 from 0x1406f73b0 with level zero and null context, converted to float. Defaults are 28000/32000 and 280/500, respectively.

Speed snapshots the phase byte before iterator construction, calls each active mutator's virtual +0x780 with the original output pointers, data pointer, and phase, and stops on any nonzero result byte. Rejected writes persist into later callbacks. Iterator destruction occurs both on acceptance and before selector 0x1419498c0 after exhaustion; its virtual context clears can have effects. The selector reads the data key only after that destruction. Invalid/non-Athena game state bypasses both iterator and selector. Spawn reads its phase and key before selector and has no iterator.

`SelectRow` is an unresolved binding including row lifetime and side effects; state and data bindings preserve the raw offsets described in the source. Do not substitute stubs or assume the callback signature's descriptive types are original names. The iterator's owner order, per-owner snapshot, active filter, context dispatch and lifecycle dependencies are described in the adjacent HANDOFF and referenced iterator proposal. No canonical entries, build or tests are claimed.

Root caller review identifies the data receiver as AFortAthenaSupplyDrop and its +0x10 selector key as UObject::GetClass(), so the rough bindings now carry UClass* rather than an opaque integer. Row selection and mutator dispatch remain unresolved.
