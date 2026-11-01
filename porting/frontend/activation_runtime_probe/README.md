# Bounded activation runtime verification

These tools extend the unchanged context verifier for the source-derived
CommonActivatableWidget base implementation and opt-in diagnostic. Exact approved
sources, both target UHT/object recipes, linked implementation/thunk bytes and
relocations, actual native metadata, fresh separate registry/lifecycle processes,
and the ordered 40-row activation transcript remain mandatory. Unsupported
relocations fail closed. Source Native c06a5cb is still runtime unverified.

Run `python3 -m unittest discover -s porting/frontend/activation_runtime_probe -p 'test_*.py'`
from the repository root. All 25 tests include actual source preflight and
synthetic tamper controls; synthetic receipts are not runtime evidence.

For a new compiled phase, use binder.py start/build/finish/run/verify through the
sole build owner with frozen inputs. For run-only attempt3, use explicit resume:

```
python3 porting/frontend/activation_runtime_probe/binder.py resume \
  --previous porting/generated/frontend-native-evidence/activation-runtime-2 \
  --parent-checkpoint porting/generated/material_pipeline/frontend-activation-runtime-failure-checkpoint-results-2.json \
  --release /mnt/vibrato/fortnite-evidence/frozen-runtime/activation-frozen-runtime-2/source-release.json \
  --out porting/generated/frontend-native-evidence/activation-runtime-3
```

Resume requires an absent output directory. It authenticates the exact released
phase2 checkpoint, frozen runtime manifest, four inherited receipt bytes, original
824af00 binder hash, unchanged audit tools/contract, current approved source tree,
Engine/generator snapshot, binary/object hashes and exact link recipes. Four
symlinks reference the original receipts; a new activation.retry-start.json pins
the parent/release, those references and current tools plus a new runtime epoch.
Full inherited artifact/ELF/debug hashes are checked once at resume and again
by final strict verification. Each run revalidates receipt/tool/source/object
recipe bindings; the runner also hashes its executable before and after launch.
Build and finish are
forbidden for a resumed attempt; no header restamping or compilation occurs.

Use `binder.py run --out ... --binary ... --cwd ...` for activation. Then use
`binder.py run-context --case registry --out ... --binary ... --cwd ...` and the
same command with `--case lifecycle`. All three runners use the proven phase2
project/Entry-map/Vulkan/windowed/unattended/compatibility startup baseline with
fixed diagnostic flags; no caller startup/phase extras are accepted. Every retry
process must start at or after the new retry epoch, and final verification checks
exact argv for all three. Existing staged executable hashes must match build2.

Finally use `binder.py verify --out ... --original ... --context-report ... --report ...`.
The unmodified complete context verifier, both target metadata audits, exact
implementation/link audits and activation transcript verifier all remain required.
The report labels the build as inherited immutable phase2 and pins the new retry
receipt; runtime attempt3 never claims a new compile. Old failed receipts remain.

External original/source receipts must remain available. Exhaustive hermetic
loader/toolchain/library/asset closure, original private ABI, actual cooked
Blueprint behavior and product milestone acceptance remain outside this bounded
diagnostic. The earlier startup stall cause remains unproven; these tools claim
no activation runtime pass.

The attempt3 metadata audit exposed two symbols ending in `v`: the UFunction
constructor and the bool setter ending in `EPv`. Constructor selection now
matches the complete no-argument constructor name. The old phase2 metadata
observer hash remains explicitly recognized as historical evidence; the corrected
observer is separately pinned in each retry and still performs every object and
linked metadata check. Native/generator/Engine bytes are unchanged. The real
ambiguous pair and missing/duplicate-constructor rejection have regression tests.
