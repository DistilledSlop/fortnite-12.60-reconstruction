# Bounded activation runtime verification

These tools extend the unchanged context runtime verifier for the source-derived
CommonActivatableWidget base implementation and its opt-in diagnostic.
They require exact approved source, both fresh target builds, fresh UHT/object
recipes, exact implementation/thunk bytes and relocations in the linked ELF,
actual native metadata, separate context registry/lifecycle runs and the ordered
40-row activation transcript. Unsupported relocations fail closed.

Run `python3 -m unittest discover -s porting/frontend/activation_runtime_probe -p 'test_*.py'`
from the repository root. The 19 tests include actual generated CPP/header labels
and complete source preflight; synthetic ELF/receipt controls are not live proof.

Use binder.py start/build/finish/run/verify only through a sole build owner with
frozen inputs. Contract authorities include external immutable original/source
receipts; keep them available. `source-expectations.json` is the approved six-file
context extension. The first generated comment permits a different reflection
path only; CPP's fixed second sentence and all source body bytes remain exact.

Google runtime phase2 at Native5d45ea2 remains the prior green baseline.
The first activation attempt failed before any build because its predecessor
comment matcher omitted the CPP sentence. Preserve that failure. These maintained
tools correct that matcher without changing Native/generator/Engine source.
Exhaustive hermetic loader/toolchain/library/asset closure, original private ABI,
actual cooked Blueprint behavior and product milestone acceptance are outside
this diagnostic evidence. No activation runtime pass is claimed by tool tests.
