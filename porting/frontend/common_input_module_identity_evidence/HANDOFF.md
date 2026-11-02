# Module registration and transient outer identities closed

The static registration initializer `1401c7e90` directly binds the module name `CommonInput` to factory `14081bef0` in the module manager's pending registration array. The factory-name uncertainty in prior evidence is now closed. The module startup's outer getter `142e80820` is also identified as `GetTransientPackage()`: it returns global `148261550`, whose initialization constructs a `Package` named `/Engine/Transient`, roots it, and whose shutdown removes that root and clears the same pointer.

Original image: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, base `140000000`. No full-image hash was computed.

## Direct CommonInput registration and logging category

Root supplied the bounded static-initializer candidate; no further broad sweep was used. `1401c7eb3` loads UTF-16 `CommonInput` at `145a0d5b8`; `1401c7eba` sets length 11 and `1401c7ec4` computes its name hash. The stack representation combines the literal address with the top bit and retains hash/loaded state, matching `FLazyName` rather than a plain `FName`.

`1401c7edb` allocates a 0x30-byte delegate instance, installs vtable `1459a43f8`, stores the registrant pointer `1480d3200` at +8, and stores **factory `14081bef0`** at +0x10 (`LEA 1401c7efd`, store `1401c7f04`). It also initializes a delegate handle at +0x20. `1401c7f11` calls manager getter `142cbc9d0` (a jump to singleton getter `142cbd480`), the same getter used by the existing `CommonInput` module lookup `1407c4ffc`/`1407c5035`.

`1401c7f16` selects manager +0x58. This is a pending registration **array**, not the final name map: `1401c7f1a..1401c7f24` increments its count at +0x208 and the insertion uses a 0x20-byte element stride. `1401c7f56` copies the 16-byte lazy name into the new element. Its delegate storage begins at element +0x10. At `1401c7f7e`, vtable +0x40 calls `140680340` to clone the initializer into that destination. Clone copies the registrant, factory, and handle (+8/+0x10/+0x20); allocator `1406834e0` uses incoming RDX as destination delegate storage and saves its allocation there. Thus the actual array element pairs `CommonInput` with a delegate retaining factory `14081bef0`; this is not merely code proximity. Delegate vtable +0x48 is `140680380`, which loads +0x10 into RAX, loads +8 into RCX, and jumps to RAX, completing the dispatch to that exact factory. The temporary initializer is then destroyed.

This exactly matches the local UE 4.26 registration source in `Core/Public/Modules/ModuleManager.h:646–655`: `FStaticallyLinkedModuleRegistrant<ModuleClass>` builds a raw initializer delegate and calls `FModuleManager::Get().RegisterStaticallyLinkedModule`; `InitializeModule` at 667–670 returns `new ModuleClass()`. Registration at 369–371 appends to `PendingStaticallyLinkedModuleInitializers`; `Core/Private/Modules/ModuleManager.cpp:1102–1116` later processes pending entries and inserts them into `StaticallyLinkedModuleInitializers`. The executable windows directly prove the pending array binding and callable factory; the final map transfer is identified by this source pattern, not independently traced in machine code here.

Adjacent initializer `1401c7e20` constructs the log category at **`1480d3208`**. Its literal is exactly **`LogCommonInput`** at `1459eb748`; `1401c7e43` passes default verbosity 5 and `1401c7e3b` passes compile-time verbosity 7 to category constructor `142c475f0` at `1401c7e4d`. The category global is separate from the registrant object at `1480d3200`.

## Original-image chain

- `142e80820`: `mov rax,[rip+53e0d29]; ret` reads `148261550`.
- `142e839be` loads the UTF-16 literal at `146619cc8`, exactly `/Engine/Transient`, into RDX. `142e839c9` constructs its name representation; `142e839d0` obtains the resulting eight-byte name in RBX.
- `142e839f8` calls class getter `140e88cd0`, a direct jump to `142e80640`. That getter passes literal `Package` (`146614ce2`, LEA `142e8067b`) and `/Script/CoreUObject` (`145a61098`, LEA `142e80686`) to the class registration routine `142d698b0`, and returns the cached class pointer. Thus this call supplies the CoreUObject Package class, independently of the transient-package hypothesis.
- At construction `142e83a24`, RCX is that class, EDX is zero outer, R8 is the name in RBX, and R9D is `0x40`. `142e83a29` stores the returned object in `148261550`; `142e83a30..142e83a68` finds its global object item and sets root bit `0x40000000`.
- `142e8dcc0` reads the same global; the guarded cleanup reaches `142e8dd0c` to clear root bit `0x40000000` and `142e8dd13` to store zero into that same global.

This uniquely matches the local UE 4.26 source lifecycle in `/opt/fortnite/UnrealEngine4260/Engine/Source/Runtime/CoreUObject/Private/UObject/Obj.cpp`: global declaration line 69; `StaticUObjectInit` creates `NewObject<UPackage>(nullptr, TEXT("/Engine/Transient"), RF_Transient)` and roots it at lines 4416–4417; shutdown unroots and clears it at 4453–4456; `GetTransientPackage` returns it at 4559–4562. `Public/UObject/ObjectMacros.h:489` assigns `RF_Transient = 0x00000040`. This source comparison identifies semantics; it does not establish that the captured executable was built from that exact source revision.

## Bounded discovery and replay

The distinct approach was to trace the known outer getter's global writes. A read-only mmap search located exact UTF-16 `/Engine/Transient` at `146619cc8` (no ASCII copy), and searched RIP-relative `48 89 /r` store encodings in the first executable section, validating each candidate displacement against `148261550`. The only matching stores were `142e83a29` (`48 89 05`) and `142e8dd13` (`48 89 1d`). Only the candidate windows above were then disassembled. This is a global setter route, not a factory-address qword scan or speculative call expansion.

Replay from repository root:

```
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_input_module_identity_evidence/replay.py
```

The replay uses bounded `pread` windows, prints registration/delegate/class/name/lifecycle instructions and the delegate vtable, and checks the three global references and five exact literals. Captured output is `windows.txt`. The startup window is included only to replay its established call to the outer getter; the prior module/settings evidence owns the full startup arguments and lifecycle analysis.

No native, build, or generator changes. Both identity gaps assigned here are closed. The now-proven registration and transient outer permit the prior module factory/startup/storage/lifecycle findings to be attributed to CommonInput. They do not license replacing the newly created settings object with a CDO. Remaining settings initialization or frontend implementation work belongs to the primary investigation.
