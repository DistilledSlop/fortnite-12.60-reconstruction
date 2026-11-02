# Settings cache source candidate

These unintegrated fragments alias original validation `14081d830` and
initialization `14081c1e0`. They depend on the constructor candidate's shared
`NativeInputDataLoaded` and the reflected settings/platform members. No module
getter or call site is supplied. Integrating them requires the platform and
settings constructor policies, appropriate class and `UObject/UObjectArray.h`
includes, a shared CommonInput module definition of `LogCommonInput`, and
proof of the module factory/name link.

Validation ANDs the existing byte rather than recomputing it. The soft-pointer
check is `!IsPending()` (the target UE implementation tests the underlying
object and path), so a null path passes and an unresolved nonempty path fails;
this is distinct from typed `TSoftClassPtr::IsValid()`. Membership performs
ordered, non-consuming exact typed class-pointer comparisons, including a null
match, resolving the soft pointer separately on each comparison. Both loops use
snapshot copies and per-entry copies. Initialization samples
`GUObjectArray.IsDisregardForGC(this)` once before loading, loads the typed
InputData, conditionally roots it, copies the first complete platform struct,
then loads and appends every successfully typed class from a soft-list
snapshot and conditionally roots each. The loaded byte is set even on failures.
Existing copied class entries are not separately rooted. Duplicate entries are
preserved.

Evidence: [`platform cache`](../common_input_platform_evidence/README.md),
[`soft load and rooting`](../common_input_soft_load_evidence/README.md), and
[`settings constructor`](../common_input_settings_ctor_evidence/README.md).
Bounded decoding additionally confirms membership `140818f70..1408190d0`,
validation `14081d830..14081db61`, and initializer `14081c1e0..14081c751`.
At `14081db2a`, the RIP-relative LEA resolves to `1480d320c`; the separate
verbosity check reads byte `1480d3208` and requires at least Warning (3).
Static initializer `1401c7e20` constructs the category at `1480d3208`
from the UTF-16 name `LogCommonInput` at `1459eb748`; constructor
`142c475f0` uses default verbosity Log (5) and compile-time verbosity All
(7). See [`module identity evidence`](../common_input_module_identity_evidence/README.md).
The fragment declares this shared category; the eventual CommonInput module must
define it once. Batch integration must verify that the target build retains the
warning. The original warning text, including its spelling, is preserved. No
native, generator, build or runtime changes were made and no runtime parity is
asserted.
