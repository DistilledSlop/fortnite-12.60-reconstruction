# Behaviour snippets (porting/behavior)

Hand-ported native function bodies, taken from the decompiled output of the shipping client
(`porting/generated/decomp/` and `porting/generated/decomp_improved/`, both git-ignored). Each `.inc`
is our own C++ re-expression of what the decompile does, not a copy of it.

Status (see `MANIFEST.json`): 21 snippets ported, 78 candidates skipped with a reason. Of the 21,
3 are declared in the generated headers today and 18 need a declaration that the generator does not
emit yet. `python3 porting/behavior/check_behavior.py` reports 0 failures.

## Files

- `MANIFEST.json`: array of entries `{class, function, kind, signature, signature_status, inc_path,
  confidence, address, status, proxy_score, decompile_source, notes}`.
  - `status`: `ported` (has an `.inc`) or `skipped` (`inc_path` is null, `notes` holds the reason).
  - `kind`: `getter` (Get/Is/Has predicates and accessors), `onrep` (RepNotify body), `impl`
    (RPC `_Implementation`), `validate` (RPC `_Validate`), `exec` (other BlueprintCallable body).
  - `signature_status`: `declared` (the signature is copied verbatim from the generated header) or
    `proposed` (not in the header yet; the generator must emit it) or `not_declared` (skipped entries).
- `<ClassName>/<FunctionName>.inc`: one snippet per ported entry.
- `check_behavior.py`: stdlib checker (see below).

## The .inc contract

- A `.inc` file is the body only: the statements that go between the braces of the native member function.
  No signature, no braces around the whole body, no `#include` lines.
- It runs inside the generated member function, so `this` members are available as bare names
  (`LocalSpin`, `FootstepBank`) and `this->X` is valid. Parameters use the names from the signature.
- Non-void functions use `return`. Void functions just end with their statements.
- Members are looked up in the class and in its generated ancestors (for example `FootstepBank` is declared
  in `AFortPawn.h` and used from `AFortPlayerPawn`). All members used are `public` in the generated headers.
- Includes: the snippets need no extra includes beyond the class header. Any future snippet that
  dereferences a type must have that type's header included by the generator (the generator emits the
  `#include` for the declaring header of each type it uses).
- Each file starts with a `//` header comment: original address, decompile source, proxy score, what was
  inferred (field, offset, enum width) and `CONFIDENCE: HIGH|MEDIUM|LOW`. The checker requires the tag to match the manifest.

## Splice instructions for gen_native_classes.py

Not implemented yet. The generator needs these steps (no step changes an emitted file except as listed):

1. Read `porting/behavior/MANIFEST.json`. Use only entries with `status == "ported"`.
2. Declarations. For each entry with `signature_status == "proposed"`, emit `<signature>;` in the public
   section of `<class>.h` (add `UFUNCTION(...)` with the specifier the generator's policy uses for
   BlueprintCallable natives). Today `build_functions_promoted` in `gen_native_classes.py` (line 1122)
   drops every function that is neither an RPC nor a RepNotify target
   (`if not is_rpc and name not in notify_names: continue`), so these declarations are never emitted.
   The 18 pending getters need this change. The two `_Implementation` snippets and
   `OnRep_FootstepBankOverride` are already declared, so they need only step 3.
3. Bodies. In `<class>.cpp`, find the definition whose signature equals `signature` (normalised
   whitespace, trailing `;` removed). Replace the stub body (the `// Body not recovered` block) with the
   `.inc` contents, keeping the signature line unchanged. Add `#include`s only if the snippet needs a type
   that the class header does not already include.
4. RPCs. Port the **native** bodies: `Foo_Implementation` and `Foo_Validate`. Never put a snippet into the
   UHT exec thunk (`execFoo`, which pops `FFrame` parameters and calls the native function). The thunk stays
   generated. The current ported RPC bodies are `ServerStartLoadingVolume_Implementation` and
   `ServerStartUnloadingVolume_Implementation`. Validate entries are not ported yet: the generator emits no
   `_Validate` declaration and no `WithValidation` specifier, so it must add both before a `validate` entry
   can be spliced.
5. Non-RPC exec functions. The snippet is the native body (for example `bool IsSpinning() const`), not the
   exec thunk. Keep the thunk generated as before; it calls the native function.
6. After generation, re-run `python3 porting/behavior/check_behavior.py`. It must report `FAIL: 0`.
   `PENDING_DECL` lines are expected until the generator emits the proposed declarations.

## Checker

`check_behavior.py` (stdlib, run from anywhere: `python3 porting/behavior/check_behavior.py [--verbose]`):

- signature: `declared` entries must appear in the generated header; `proposed` entries must not already be
  declared (reported as `PENDING_DECL`, not as a failure);
- `.inc` exists; CONFIDENCE tag matches the manifest; original address present; no TODO/FIXME/TBD/XXX/
  placeholder/"not recovered"/"..." text in the file;
- best-effort token check: each bare identifier and each `this->X` in the body must be a parameter, a local, a
  type (generated header or file), a function, or a member declared in the class or a generated ancestor header.
  Member access on other objects (`obj.X`, `obj->X`) and `::`-qualified names are not checked;
- skipped entries have a reason and no `inc_path`.

Limits: the token check is textual, so it can miss a wrong member that happens to exist elsewhere in the class chain.
It does not check types or the decompile-to-C++ meaning; those are the review's job.

## Why most candidates were skipped

Summary of the reasons in `MANIFEST.json` (`notes`):

- `bitfield_or_unmapped_offset`: the body reads a byte that is not a reflection property. Bitfield `bool : 1`
  members have no offset in the reflection data (only a `set_bit_fn`), so their bit position cannot be mapped.
  This covers most of the Balloon/parachute/DBNO predicates (`IsDBNO`, `IsSkydiving`, ...). The `set_bit_fn`
  disassembly would give the masks, but it was not verified (the on-disk exe disassembled to unrelated bytes).
- `unnamed_call`: the body calls a `FUN_*` helper that is not identified.
- `rpc_exec_thunk`: exec thunks of RPCs. Their native bodies are the separate `_Implementation`/`_Validate`
  records, and most of those call unnamed helpers.
- `folded_identical_body`: the linker folded identical code (for example the `OnRep_` bodies that share
  address `0x14075b6b0` with `ToggleGender`).
- `vtable_call`, `jump_table_or_thunk`, `global_constant`, `ambiguous_offset`: see the notes.

Screen of all 750 `portable` and `needs names` records in `porting/generated/decomp/portability.json`
(same rules as the manifest notes): 21 ported; 372 `unnamed_call`; 163 `rpc_exec_thunk`; 112 `vtable_call`;
37 `bitfield_or_unmapped_offset`; 37 `folded_identical_body`; 3 `global_constant`; 2 `jump_table_or_thunk`;
2 `other`; 1 `ambiguous_offset`. The 78 manifest skips are the top-60 candidates and the RPC
`_Implementation`/`_Validate` records that were screened. The other records are counted here only.
