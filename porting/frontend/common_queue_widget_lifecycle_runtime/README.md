# Queue widget acquisition and removal

This component reconstructs the container acquisition 0x1407aa4c0 and
registration 0x1407ecbe0, queue virtual slot +0x2c0 0x1407dfda0, and removal
0x1407f1000. Acquisition uses the owned `FUserWidgetPool` and its standard
SObjectWidget construction; the supplied synchronous initializer runs before
unique registration and the virtual addition callback. Queue addition inserts
`TakeWidget()` at switcher slot 1, transitions to slot 1 normally when there
are two slots, or instantly when there are three and the displayed widget is
inactive. The base virtual slot is the original empty function 0x1406801c0.
Removal checks the switcher's active Slate object, deactivates it if active or
sets pending deactivation if already inactive; other widgets pass their cached
Slate object to the integrated pool/list/slot removal path when available.

Original base vtable 0x1459fd6c0 +0x2c0 is 0x1406801c0; queue vtable
0x145a03d58 +0x2c0 is 0x1407dfda0. Original pool entry 0x140785860 and local
UE 4.26 UserWidgetPool.h support the standard GetOrCreateInstance call.
`policy.py` pins bounded original instruction and vtable windows and requires
both models to have passed the existing animated-switcher policy. Integrators
must call its apply, configure_model, declarations, definitions, sources and
validate_output alongside that policy, and include the exported header for
cross-module callers. Initializer takes a pointer and is invoked synchronously.
The existing native switcher index callback owns activation, delegate binding,
list/pool release, deferred Slate lifetime and the deactivation delegate.

Static reconstruction only; no generated source, UE build or runtime test was
performed. The underlying pool and activation implementations remain required.
