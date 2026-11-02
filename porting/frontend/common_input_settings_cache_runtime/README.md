# Settings cache runtime policy

This policy installs source aliases for the original validation (14081d830),
initialization (14081c1e0), and current-platform copy (14081b550). The public
`NativeGetCurrentPlatformData()` is an instance method: callers first obtain
and retain the actual CommonInput module settings pointer, then invoke the method.
It validates on that object and returns its `CurrentPlatform` by value. It does
not itself implement the module getter or substitute a class default object.

The existing constructor policy must run first: its `NativeInputDataLoaded` is
the only cache-state field. Validation preserves the snapshot and per-entry copy,
repeated typed class resolution, ordered membership, pending tests and warning.
Initialization samples the disregard-for-GC predicate before loads, roots
successful typed loads conditionally, copies the first entire platform record,
and appends every successfully loaded class to the copied array. It sets the
shared flag even on failures. See the sibling `common_input_platform_evidence`,
`common_input_soft_load_evidence`, and `common_input_settings_ctor_evidence`
for the recovered ordering and boundaries. `LogCommonInput` is declared here
and must be defined once by the CommonInput module.

Generator integration must run `validate_source(image_path)` against the mapped
original image and `validate_classes(blocks, name_to_block)` and
`validate_types(gate)` before output. Run `validate_platform_model(info, model)`
after applying the platform constructor policy; run `apply(model, block,
owner_fns)` after the settings constructor policy. Append `declarations(model)`
and `definitions(model)` to the generated class and source, add
`UObject/UObjectArray.h` to the source includes, and call
`validate_rendered(model, header, source)` on the final strings before either
file is written. These checks cover the bounded original code and loading
windows, exact reflected metadata and relevant generated types/members, shared
state, and emitted cache fragments. They do not establish full startup parity.
