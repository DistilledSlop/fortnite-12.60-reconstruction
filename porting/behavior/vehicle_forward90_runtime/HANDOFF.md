# Vehicle secondary-interface dispatch recovery

No accepted candidate or manifest entry: all five AFortAthenaVehicle declarations are absent from the generated class, and the Win64 virtual calls are through a secondary interface at actor+0x3b0. Adding forwarding wrappers without recovering and binding their interface implementations would hide native work and lose derived-class dispatch. No `.inc` drafts are proposed.

Original exec probes: `porting/generated/native89-broad/0x14283ac10.c`, `0x14283ac40.c`, `0x14283ab50.c`, `0x14283cbd0.c`, `0x14283ccf0.c`. Constructor resolver `python3 -I` importing `porting/behavior/build_vtable_slots.py` resolves AFortAthenaVehicle GPSC 0x1428381e0 to constructor 0x141908660. Constructor instructions 0x1419086fa–0x141908701 store secondary vtable 0x145decf00 at actor+0x3b0. Reading five entries from the dumped PE via the same script's `Image.u64` yielded:

| Function | secondary slot | implementation entry |
| --- | --- | --- |
| DestroyVehicle | 0x30 | 0x141911a70 |
| EjectPlayerPawn | 0x330 | 0x1419124d0 -> 0x141f860c0 |
| EjectAllPlayers | 0x338 | 0x1419124c0 -> 0x141f860a0 |
| OverrideCurrentWrap | 0x3d0 | 0x14097d780 -> 0x1406801c0 (empty implementation) |
| ResetTrick | 0x3e8 | 0x14084efc0 -> 0x141f96c30 |

The Eject methods call secondary vtable slot 0x410, whose concrete entry 0x14084c920 loads `[interface+0x740]` = `[actor+0xaf0]`; their tail calls are 0x141d04ae0 and 0x141d04a20 respectively. Recover and bind these concrete methods, identify the typed interface and its inheritance, and preserve virtual dispatch for subclasses before accepting bodies. ResetTrick also calls slot 0x3a0 (0x14084c840) and requires its contract. An empty OverrideCurrentWrap implementation is still insufficient to replace the original virtual call, since subclasses can override it. The generated class headers strip the relevant secondary interface and do not currently declare these methods.

Searches: `rg -n 'EjectAllPlayers|EjectPlayerPawn|OverrideCurrentWrap|ResetTrick|DestroyVehicle' porting/behavior porting/generated/native89-broad --glob '*.inc' --glob '*.c' --glob '*.json'`, plus generated headers. No duplicate accepted bodies found. `porting/generated/reflection/function_blocks.json` records original reflected signatures; EjectPlayerPawn returns bool and takes InPlayerPawn. No build or test run, per delegation.

## Rough lift after native90 probes

`rough_native_lift.cpp` now captures DestroyVehicle, ResetTrick, EjectAllPlayers and EjectPlayerPawn including the 0x141d04a20/0x141d04ae0 loops. It explicitly preserves authority-byte branches, virtual dispatch, destructor/refcount/free ordering, live array count/data reads, controller class checks, and the eject return rules. Address-based operations and unresolved types are deliberate source-lift dependencies, not implemented helpers; it is uncounted and not compilable. DestroyVehicle's 0xc80 call receives the virtual 0x670 result in XMM0 as a float, established from assembly despite the decompiler's integer typing. OverrideCurrentWrap remains only the virtual-dispatch evidence above. No test/build performed.
