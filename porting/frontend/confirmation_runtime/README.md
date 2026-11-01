# Confirmation description runtime

This policy restores `FFortDialogDescription_NUI` construction and the two native
single-cast delegate values omitted from reflection. It guards the original
description, result enum and latent-action handle records before emitting code.
The native member identifiers are project-local names.

Original struct ops report size 280, alignment 8, nonzero construction, destruction
and C++ copy. Construct thunk `142b1d5f0` calls constructor `142b1bea0..142b1bfb7`.
The brush, text, array, name and delegates use their actual default constructors;
only recovered pointer/float/bool defaults are explicitly initialized. Native
callbacks occupy offsets 248 and 264. The destructor and complete copy CFG manage
both independently. No padding bytes are assigned a semantic value.

The ordinary callback contract is `(EFortDialogResult, FName)` by value. The
advanced action binds its second callback in `144ae8950..144ae8b90`; its callback
`144afc7c0` forwards result, name, bool and `FFortDialogExternalLatentActionHandle`
to the reflected advanced multicast. The `ProcessConfirmationResult` reflected
exec independently agrees on these types. The native delegate's void return is
inferred from its concrete `ExecuteIfSafe` implementation and UE's corresponding
void specialization, rather than from a discarded return register alone.

Hash-only evidence is in `original-runtime.json`. Recheck the immutable input:

```
python3 porting/frontend/confirmation_runtime/verify_original.py /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin
```

Detailed original disassembly/replay scripts remain in the scratch evidence
directories `confirmation-description-ctor-1` and `confirmation-tail-signature-1`
under `/mnt/vibrato/fortnite-work/`. Copy and Activate ranges can span adjacent
PE unwind records; they are explicitly complete CFG windows in the hash record.

Portable tests exercise policy guards and generated declarations using mocked
delegates. They do not establish UE ABI or ownership. The opt-in
`-ConfirmationDescriptionProbe` exercises actual generated members and Unreal's
construction/copy/destruction machinery. Neither check proves action factory,
activation, interactive presentation, Blueprint execution or pixel parity.
