# Remaining native function bodies: classification and required work

Generated 2026-11-07 by `porting/classify_remaining.py` (read-only; rerun it for current numbers; output `generated/remaining/classification.json`).
Population: the 10,255 in-scope functions of the thunk table (`final2.json`) that are not ported (4,978 table rows are ported). Every function is in exactly one group, the first rule that matches.
Size limit for "ready" bodies is 60 instructions. Counts are functions.

| # | Group | Functions | What it means | What needs to be done |
|---|---|---:|---|---|
| 30 | Ready | 98 | Single captured body, clean thunk, every call target resolvable | Run as worker batches (`build_call_pool.py` -> `build_job_batches.py`). No new tooling. |
| 03 | Inline thunk (group A), no separate body | 758 | The exec thunk's own decompile is the body | Port from the thunk decompile (`generated/native*/<thunk>.c`). Needs worker rules for decompile-sourced ports (receiver is `this`, parameters come from the thunk's stack frame, not the register ABI). This group has never been run. |
| 04 | Several candidate bodies (group B) | 694 | The thunk calls 2 to 26 different bodies | Pick the real body per function (ground truth says the first call is not always the body). Needs a disambiguation pass: match receiver, argument count and return use against the thunk. Output feeds groups 30/20/21/22. |
| 12 | Hand-written class with no generated header | 298 | `UFortKismetLibrary` has no generated header, so the generator cannot learn signatures (18 of 28 probe jobs in policy-class tests were rejected, all Kismet). | Decide how Kismet entries are declared (hand-written header edits) or add generation for the class. |
| 00 | Already emitted by a generator policy | 56 | A policy in `gen_native_classes.py` already emits a real body (not the reflection stub), so a manifest entry would collide. | Credit them as done (they have no manifest entry, so the dashboard does not count them). |
| 23 | Thunk does extra work | 95 | The exec thunk makes extra calls or non-`this` virtual calls besides the body | Port thunk logic and body together as one function (like group A), after checking the extra calls are only by-value destroys (those are not logic). |
| 20 | Body too large | 3,134 (61-120: 1,473; 121-300: 1,335; 301-1000: 326) | Body exceeds the call-tier limit | Not a blocker by itself; these also hit the callee and indirect-call groups. Port after the helper registry exists; larger bodies need per-function review and the decompile (`.c`), not asm. |
| 21 | Indirect calls | 1,930 | Body makes virtual or function-pointer calls | About 400 call virtuals on `this`: need the by-address slot resolver (slot read from the class vtable, matched to a reflected function body; unnamed slots stay unresolved). About 280 call through a member pointer: need the member's reflected class, then the same resolver. About 630 have unknown receiver provenance and about 65 call through a parameter: need decompile-based porting by a worker. About 35 are function-pointer/delegate calls. Thunk-based slot naming was tried and is wrong (`NATIVE_PORTING.md` 11.3). |
| 22 | Calls internal helpers not in reflection | 2,376 (1 helper: 1,320; 2: 663; 3+: 393) | Call targets are C++ functions with no reflection entry | Build the helper registry: address -> engine expression, or a generated `NativeHelper_<addr>` free function. Rank by callers: top 25 callees unblock about 211 bodies, top 100 about 390, top 200 about 529. Callees mix engine API (e.g. `0x142cd4ab0`, FString from TCHAR*) and unreflected game logic (`0x144d03d50`). Helpers must not use undeclared globals. |
| 10 | Recorded deferral from earlier worker passes | 589 | A worker or triage pass recorded the reason (`behavior/stub_bulk_runtime/*.json`, `*_runtime/dispositions.json`) | See the breakdown below. |
| 02 | No single body | 4 | Pure virtual slot or embedded subobject | Port in the implementing subclasses. |
| 01 | Interface dispatch | 0 here | Excluded from the 14,391 denominator (252 functions) | None. |

## Group 10 breakdown

| Count | Reason | What needs to be done |
|---:|---|---|
| 205 | callee not declared (190 + 15 blocked variants) | The callee is a member of the same class (or another generated class) that is not a reflected function, so the header has no declaration. Add native member declarations for these callees (port them as helper members), then re-run the caller. Largest single fix in this group. |
| 116 | branch leaves body | The body conditionally jumps out to a continuation. Capture the continuation (scripted) and port body plus continuation. |
| 68 + 5 + 2 | shared callee ambiguous / no receiver class / shared body mismatch | One shared body serves several classes. Needs a per-class receiver mapping for the shared body; the data to decide it must come from the callers' receiver types. |
| 32 | absolute / hidden globals (26 + 4 + misc) | Body reads or writes a global by absolute address. Needs a named global owner (declare the global in a generated module header) or a decision to model it. |
| 23 | real body is the tail-jump target | The recorded body is only the first hop; re-resolve to the jump target and re-run. Scripted. |
| 14 | virtual slot mismatch | Slot named wrongly; fixed by the by-address resolver. |
| 27 | other-module or missing classes (class not in native set 16, other module 5, no module 3, policy owned 6) | Add the class to `native_class_set.txt` where the module exists; the rest need module/plugin decisions. |
| ~100 | single-case reasons (struct/enum/delegate not reflected, SIMD constants, engine backport, signature or receiver mismatches, truncated body, generator gaps) | One-off per function; none repeats more than 5 times. |

## Order of work (largest verified yield first)

1. Group 30 (88): ready now.
2. Group 03 (747) and 23 (91): needs only worker rules for decompile-sourced ports; no generator change.
3. Group 10 scripted items: tail-jump target re-resolve (23), continuation capture (116), callee declarations (205).
4. Group 04 (684): disambiguation pass, which then releases bodies into 30/20/21/22.
5. Helper registry (group 22 and the helper part of 20): about 211 bodies for the first 25 callees.
6. By-address virtual resolver (group 21, about 700 bodies at best).
7. Decide how `UFortKismetLibrary` (298) is declared; credit the 56 policy-emitted functions; reconstruct CommonUI non-reflected declarations; module/plugin gaps.

## Corrections made while building this

- The generator emits a reflection stub (`Body not recovered`) for every reflected function. Any "is it defined?" check must ignore stubs.

- `build_call_pool.py` had treated 619 functions in `wrap1`-`wrap21` as already attempted. Those batches were generated but never run (no `*_runtime` directory), so the functions were silently excluded. Fixed: only batches with a runtime directory count as attempted.
- The same script treated every direct jump inside the body as an unresolved external call. It now uses the body's address span, which raised the ready pool from 21 to 88.
- Weak-pointer helpers (`0x142ee5dd0`, `0x142ee7880`, `0x142ee3b30`) are now accepted as known helpers in the pool script.

## Policy classes (corrected)

Earlier versions of this file and of the pool scripts excluded 24 whole classes (`UCommon*`, `UFortGlobalUIContext`, `UFortGlobals`, `UFortKismetLibrary` and others) as "policy-owned". That was too broad. A generator policy collides only with the specific functions it already emits (56 functions have a real body in the generated source). Probing the other 28 ready functions in those classes learned signatures for 10 and rejected 18, all `UFortKismetLibrary` (no generated header). No promotion crash occurred. The scripts now skip only functions that already have a real generated body (`generated_defs.py`) and `UFortKismetLibrary`.

## CommonUI declarations (affects about 99 CommonUI functions and CommonUI calls inside other ports)

The project's `CommonUI` and `CommonInput` modules contain reflection-generated headers (UPROPERTY/UFUNCTION members, 61 CommonUI classes). The game install has only the plugin descriptor and content for CommonUI (no headers; shipped games do not include them). The non-reflected C++ members (native virtuals such as the activation and input-mode hooks, Slate widget classes, helper methods) are in the game binary, so they can be reconstructed from the class vtables and decompiles like any other class. Until that is done, worker ports use signatures from the Fortnite 14 CommonUI source, which must be checked against 12.60 before a build.
