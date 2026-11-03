# Creative native helper recovery79

Four uncounted, non-build-input drafts, root owned:
- 1419d9d90 canonicalizes tags against runtime options, first matching entry,
  unique output, explicit string allocation/copy.
- 1419ddfd0 checks publish tag count and string lengths, retains signed tests,
  short-circuit order, logging paths and raw upper RAX provenance.
- 142705ec0 branches on text predicate, constructs/retains a temporary,
  copies 20 bytes with ordered atomic shared/weak release operations.
- 141ed1800 is the TLS-epoch guarded global constructor/atexit accessor.

Full C sources reviewed. Decompiler omitted memcpy count and logging varargs;
these stay explicit unresolved adapters. Native layouts, allocators, text storage,
atomic semantics and engine call ABI remain open. Transform temporary storage
holds at least the observed 20-byte value, without fabricated zero initialization.
Tag subtraction preserves signed-to-64-bit conversion. No accepted-body increase,
compilation, builds, tests, runtime or canonical generation.

Sources from root native-broad79 probe; sources.json pins the generated C files.
