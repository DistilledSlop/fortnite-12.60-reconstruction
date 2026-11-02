# CommonActivatablePanel HasInputActionHandler: source recovery boundary

Unregistered, unbuilt evidence only. Do not promote this function as a constant,
create replacement maps, or regard it as generator-ready: the two original
panel-owned containers and their complete producers/removal, router and legacy
manager dependencies are not yet available as maintained source. This is a
potential next missing import, not a verified Root crash location.

Reflection `UCommonActivatablePanel` block `145a00360`, native array `145a038c0`:
`HasInputActionHandler` create `1408056f0`, exec `14080ba10`, function flags
`0x54020401`; `InputActionRow` is a by-value struct parameter at offset 0,
flags `0x10000000000080`, struct constructor `1446f2880` (FDataTableRowHandle);
return is bool, flags `0x10000000000580`, params size 24. Exec advances the
script frame, copies the complete 16-byte handle and calls `1407cd110`; stores
AL in the bool result. Thus the expected source signature is
`bool HasInputActionHandler(FDataTableRowHandle InputActionRow) const` (const
from FUNC_Const), with original Blueprint function metadata retained from
reflection.

`1407cd110` validates: if DataTable pointer is null, call `142cd8ed0` with the
name comparison-index dword and zero, AND require the name number dword to be
zero; if both conditions hold log conditionally and return false. A null table
with a non-None name or a nonnull table with None name still proceeds. It looks
up the full 16-byte key in the panel map at +360 using `1407bdba0` (entry
stride 0x98); if present returns true. Otherwise it looks in the panel map at
+310 using `1407bdde0` (stride 0x20), returning true if present, false otherwise.
Lookup finds an entry, not a bound delegate or currently live router record.
Both helpers compare through `14418a050`: exact table pointer and both 32-bit
FName words. They use hashed sparse containers, with chain link offsets +90
and +18 respectively. Membership semantics, not hash iteration order, govern
this query. Key validity check happens before any map lookup.

Producer evidence: `SetInputActionHandler` exec `14080c7d0` calls `1407f7220`,
which passes the row and delegate wrappers to shared registration `1407f6930`.
`AddInputActionNoHandler` exec `14080a000` calls `1407a9f20`, which also reaches
`1407f6930` for non-null DataTable with empty delegate wrappers. The shared
registration first looks up +310; when an existing entry has a negative ID or
`1407bd730` cannot resolve it, it removes that stale entry via `1407efd40`.
When +310 has no entry, the zero fifth-argument branch builds binding arguments,
obtains the real router, calls `1407ed1c0`, appends the returned ID to the
panel +230 array, and inserts the key/ID in +310 via `14078b1b0` only if ID
is nonnegative and registry `1407bd730` resolves it. If no router exists the
ID is -1. The nonzero fifth-argument branch validates the handle and checks
+360 for an existing entry, then constructs the legacy payload and inserts into
+360 with `14078aed0` at `1407f6f65`. Exact full producer conditions, type
identity, lifecycle and effects still require closure. Public removal exec
`14080bd70` calls `1407f0d10`, which checks both containers and performs more
than mere deletion. `RemoveAllInputActionHandlers` also exists. See
`../common_action_route_registration_runtime/HANDOFF.md` for unfinished real
router registration and `../common_activatable_panel_native_events.md` for
legacy manager and +360 progress/cleanup dependencies.

Evidence source: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`
base `0x140000000`. Bounded independent disassemblies: exec `14080ba10:af`,
body `1407cd110:11a`, lookups `1407bdba0:240` and `1407bdde0:1a5`, key
compare `14418a050:26`, producer wrappers `14080c7d0:e6`, `1407f7220:110`,
`14080a000:bf`, `1407a9f20:120`, registration `1407f6930:63a`, removal
`14080bd70:90` and `1407f0d10:200`. Use
`python3 /mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py VA:SIZE`.
Reflection comes from targeted records in `porting/generated/reflection/`.
No original binary execution, generation, build, or tests performed.
