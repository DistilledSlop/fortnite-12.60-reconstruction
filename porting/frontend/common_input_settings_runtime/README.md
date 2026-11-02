# CommonInputSettings constructor candidate

This generator policy reconstructs the settings object's native
constructor and its shared, unreflected cache-valid boolean. Source instruction
windows and exact seven-record defaults are in
[`common_input_settings_ctor_evidence`](../common_input_settings_ctor_evidence/README.md).
The constructor depends on the reconstructed `FCommonInputPlatformData` constructor
in [`common_input_platform_runtime`](../common_input_platform_runtime/README.md):
all struct defaults and empty arrays must be installed before these overrides run.
The normal UObject constructor, property initialization, template copying and
config loading lifecycle must remain intact. `NativeInputDataLoaded` names the
original +0x180 state; future cache preparation must use this same field.
`CurrentPlatform` receives its ordinary struct defaults; no cache preparation is
performed here.

Integration hooks: load policy, call `validate_classes(blocks_by_va, name_to_block)`
after reflection loading, invoke `apply(model, block, owner_fns)` in promoted class
construction, and append `declarations(model)` and `definitions(model)` to the
class renderer, like the existing button style policy. Integrate the platform
struct policy first and require its validated constructor in the combined promotion.
Full original reflected class/property records and emitted member declarations are
checked. The class identity map evidence uses constructor-registration function
`14081ed60`; the separate class getter `14081df50` is part of the frozen original
class record. No generation, build, cache loading, module setup, or runtime
validation has been performed in this candidate.
