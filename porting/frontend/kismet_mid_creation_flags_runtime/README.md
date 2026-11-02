# Material creation flags Blueprint ABI backport

Engine commit c945a157c adds EMIDCreationFlags and the fourth CreationFlags
parameter to KismetMaterialLibrary.CreateDynamicMaterialInstance, including the
matching 4.26 outer-selection/Transient behavior. Apply engine.patch from the
UnrealEngine checkout root; preserve unrelated Engine edits.

The native 7a66a401 / Engine87acab901 Root retry reaches ErrorLlama_C during
NativeConstruct. GDB records its ExecuteUbergraph_ErrorLlama call at code offset270
to Engine.KismetMaterialLibrary.CreateDynamicMaterialInstance. At offset312 the
VM consumes a leftover zero-valued byte argument as EX_LocalVariable and reads
EndFunctionParms/next instructions as a property pointer. The 4.25 reflected
function has three inputs; the supplied call and local 4.26 reference have the
fourth CreationFlags input. This is a native function marshaling mismatch.

Evidence: generated/button-production-batch-1/root-debug-6/{gdb.log,result.json},
with node/context/class, native call candidates, offset and bounded bytes. Earlier
GDB attempts retain the known Vulkan ensure, unavailable expressions and basic
stack/name evidence; root-debug-6 supplies the actionable callsite.

The backport uses the local UnrealEngine4260 header/cpp contract: None=0 and
Transient=1, with flag-dependent persistent-world-context versus transient outer,
normal null-parent return and original MID factory/flag setting. Existing callers
use the None default. Native function declaration and implementation change
coherently; VM pointer reads and cooked scripts are unchanged.

At this checkpoint Engine c945a157c is committed but unbuilt. The previous
client/server/joins baseline uses Engine87acab901. Build the client and retry
actual Root attachment, then the affected integration checks/server build/joins
before accepting the new Engine baseline. This does not establish pixel/audio
or full feature parity.
