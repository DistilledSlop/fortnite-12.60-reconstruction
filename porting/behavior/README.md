# Behaviour snippets (porting/behavior)

Hand-ported native function bodies, taken from the decompiled output of the shipping client
(`porting/generated/decomp/` and `porting/generated/decomp_improved/`, both git-ignored). Each `.inc`
is our own C++ re-expression of what the decompile does, not a copy of it.

Status (see `MANIFEST.json`): 36 snippets ported (27 high, 9 medium confidence), 64 candidates skipped with a
reason. Of the 36, 5 are declared in the generated headers today and 31 need a declaration that the generator does
not emit yet. `python3 porting/behavior/check_behavior.py` reports 0 failures.

## Files

- `MANIFEST.json`: array of entries `{class, function, kind, signature, signature_status, inc_path,
  confidence, address, status, proxy_score, decompile_source, notes}`.
  - `status`: `ported` (has an `.inc`) or `skipped` (`inc_path` is null, `notes` holds the reason).
  - `kind`: `getter` (Get/Is/Has predicates and accessors), `onrep` (RepNotify body), `impl`
    (RPC `_Implementation`), `validate` (RPC `_Validate`), `exec` (other BlueprintCallable body).
  - `signature_status`: `declared` (copied verbatim from the generated header), `proposed` (not in the header
    yet; the generator must emit it), or `not_declared` (skipped entries).
- `<ClassName>/<FunctionName>.inc`: one snippet per ported entry.
- `bool_bits.py`: maps every bool property to its byte offset and bit mask from its SetBit function. It imports
  `porting/class_checksum.py` (read only) and writes `porting/generated/reflection/bool_bits.json` (git-ignored).
  All 6,576 bool setters decode. The map cross-checks the decompile: `IsDBNO` reads byte +0x53a bit 0x2, and the
  setter for `bIsDBNO` is `or byte [rcx+0x53a], 2`.
- `BLOCKING_CALLEES.md`: ranked list of the unidentified `FUN_*` callees that block the most functions, with
  guesses from each callee's own code, and the RPC thunk / vtable-call blockers.
- `check_behavior.py`: stdlib checker (see below).

## The .inc contract

- A `.inc` file is the body only: the statements that go between the braces of the native member function.
  No signature, no braces around the whole body, no `#include` lines.
- It runs inside the generated member function, so `this` members are available as bare names
  (`LocalSpin`, `bIsDBNO`, `FootstepBank`) and `this->X` is valid. Parameters use the names from the signature.
- Non-void functions use `return`. Void functions just end with their statements.
- Members are looked up in the class and in its generated ancestors. All members used are `public`.
  Bitfield bools are read and assigned by their header name (`uint8 bIsDBNO : 1;`), for example `return bIsDBNO;`.
- Includes: the snippets need no extra includes beyond the class header. Any future snippet that dereferences
  a type must have that type's header included by the generator.
- Each file starts with a `//` header comment: original address, decompile source, proxy score, what was
  inferred (field, offset, mask, enum width) and `CONFIDENCE: HIGH|MEDIUM|LOW`. The checker requires the tag to match the manifest.

## Splice instructions for gen_native_classes.py

Not implemented yet. The generator needs these steps (no step changes an emitted file except as listed):

1. Read `porting/behavior/MANIFEST.json`. Use only entries with `status == "ported"`.
2. Declarations. For each entry with `signature_status == "proposed"`, emit `<signature>;` in the public
   section of `<class>.h` (add `UFUNCTION(...)` with the specifier the generator's policy uses for
   BlueprintCallable natives). `build_functions_promoted` in `gen_native_classes.py` (line 1122) drops every
   function that is neither an RPC nor a RepNotify target (`if not is_rpc and name not in notify_names: continue`),
   so these declarations are never emitted today. The 31 pending getters and predicates need this change. The two
   `_Implementation` snippets and the three `OnRep_` snippets are already declared, so they need only step 3.
   A getter that returns an enum needs that enum's header included (`EFortControlRecoveryBehavior.h`,
   `EFortPawnStasisMode.h`), and the pointer returns need the forward declarations.
3. Bodies. In `<class>.cpp`, find the definition whose signature equals `signature` (normalised
   whitespace, trailing `;` removed). Replace the stub body (the `// Body not recovered` block) with the
   `.inc` contents, keeping the signature line unchanged.
4. RPCs. Port the **native** bodies: `Foo_Implementation` and `Foo_Validate`. Never put a snippet into the
   UHT exec thunk (`execFoo`, which pops `FFrame` parameters and calls the native function). The thunk stays
   generated. Validate entries are not ported yet: the generator emits no `_Validate` declaration and no
   `WithValidation` specifier, so it must add both before a `validate` entry can be spliced.
5. Non-RPC exec functions. The snippet is the native body (for example `bool IsDBNO() const`), not the exec thunk.
   Keep the thunk generated as before; it calls the native function.
6. After generation, re-run `python3 porting/behavior/check_behavior.py`. It must report `FAIL: 0`.
   `PENDING_DECL` lines are expected until the generator emits the proposed declarations.

## Checker

`check_behavior.py` (stdlib, run from anywhere: `python3 porting/behavior/check_behavior.py [--verbose]`):

- signature: `declared` entries must appear in the generated header; `proposed` entries must not already be
  declared (reported as `PENDING_DECL`, not as a failure);
- `.inc` exists; CONFIDENCE tag matches the manifest; original address present; no TODO/FIXME/TBD/XXX/
  placeholder/"not recovered"/"..." text in the file (checked on the raw text, comments included);
- best-effort token check: each bare identifier and each `this->X` in the body must be a parameter, a local,
  a type (generated header or file), a function, or a member declared in the class or a generated ancestor header.
  Member access on other objects (`obj.X`, `obj->X`) and `::`-qualified names are not checked. Numeric literal
  suffixes (`0.0f`) are skipped;
- skipped entries have a reason and no `inc_path`.

Limits: the token check is textual, so it can miss a wrong member that happens to exist elsewhere in the class chain.
It does not check types or the decompile-to-C++ meaning; those are the review's job.

## How the bit masks were found, and what is still missing

Reflection gives no offset for `bool : 1` members (only a SetBit function). `bool_bits.py` decodes the SetBit
code in the runtime dump (`or byte ptr [rcx+disp], mask; ret`), which gives byte offset and mask. A getter reads
a candidate bit only if its raw decompile masks that byte with one bit (for example `>> 1 & 1`) and the
(offset, mask) pair maps to exactly one bool in the class chain. Whole-byte reads are accepted only when the
byte holds one bool that is written with a whole-byte `mov` (medium confidence).

Screen of all 750 `portable` and `needs names` records (`porting/generated/decomp/portability.json`, same rules
as the manifest notes): 36 ported; 372 `unnamed_call` (includes 8 records whose only call is an identified FFrame
helper); 164 `rpc_exec_thunk`; 111 `vtable_call`; 37 `folded_identical_body`; 12 `unmapped_offset`;
10 `whole_or_multibit_byte_read`; 3 `global_constant`; 2 `jump_table_or_thunk`; 2 `other`; 1 `ambiguous_offset`.

- `unnamed_call`: the body calls a `FUN_*` helper that is not identified. `BLOCKING_CALLEES.md` ranks them.
- `rpc_exec_thunk` and `vtable_call`: exec thunks of RPCs, and thunks that call a vtable slot. Their native bodies
  are the `_Implementation`/`_Validate` records. Most of those records call unnamed helpers, and 197 named RPC
  targets have no decompile record at all (listed with dump addresses in `BLOCKING_CALLEES.md`).
- `folded_identical_body`: the linker folded identical code (for example the `OnRep_` bodies that share
  address `0x14075b6b0` with `ToggleGender`).
- `whole_or_multibit_byte_read`: the byte holds bool bits, but the body reads several bits, the whole byte in a
  non-flag way, or a pointer chain.
