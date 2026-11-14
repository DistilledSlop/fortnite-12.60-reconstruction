# Shared profile backend query dispatch

The complete original UMcpProfile wrapper 1410fe3b0 uses the actual shared backend pointer and dynamic base/overlay dispatch. Its original constructor sets this pointer to null. Installation, mutations, destruction and full GC remain unported; this source lift does not establish runtime availability.

## Proven original identity and fields

Original UMcpProfile ctor 0x1410ac0f0 explicitly stores zero at profile +0xd8. Its source is `callee_probe/profile_backend_installation/0x1410ac0f0.c`. This justifies one real native member `FMcpProfileBackendBase* NativeBackend = nullptr`, rather than an eagerly allocated backend or fabricated default base instance. It is a member_field contract analogous to the accepted weapon stat-cache fields, not a complete UMcpProfile constructor lift.

Original installer 0x14110e120 deletes the prior pointer, chooses allocation size 0x198/0x338 from profile byte +0x92, calls base ctor 0x1410ec490, installs derived table when selected, and stores the result at the SAME +0xd8. Base ctor installs 0x145bfcc38; derived installation installs 0x145bfcca8 after base initialization. Thus the derived identity shares the actual base subobject/indexes; it is not an unrelated strategy with parallel state. The original names are unknown: FMcpProfileBackendBase/Overlay are neutral semantic source names keyed to these proven identities, not claimed original RTTI names.

Read directly from cached flat image, table slots are:

| Slot | Original base table 0x145bfcc38 | Original overlay table 0x145bfcca8 |
|---|---|---|
| +0x00 | 0x1410efb60 | 0x1410efba0 |
| +0x08 | 0x1410f0e40 | 0x1410f1060 |
| +0x28 | 0x1410fe1c0 | 0x1410fe220 |
| +0x30 | 0x1410fe3c0 | 0x1410fe440 |
| +0x38 | 0x1410fdd30 | 0x1410fde50 |

The base group query reads backend +0x58; item query reads +0x08. Derived group query uses inherited base +0x58, derived cached groups +0x298, dirty groups +0x2e8 and override projection +0x198 through its recompute. Derived item query first looks for presence in +0x198, falling back to SAME inherited base +0x08 only on key absence. Existing McpProfileBaseTypes and ProfileOverlayState are the semantic representations already admitted for these exact components. The larger native override records remain only partially represented; do not invent their missing fields or claim the partial storage closes their writers.

Wrapper assembly is exactly three instructions: load RCX from [RCX+0xd8], load RAX from [RCX], jump [RAX+0x28]. Key passes unchanged. `return NativeBackend->FindItemsByTemplateKey(Key);` is the complete semantic lift. There is no null guard, static-base default, new map, synthesized empty group or fallback allocation.

## Reconstructed source contract

`McpProfileBackend.h/.cpp` are emitted in McpProfileSys by the existing profile-entry source policy. The native classes use the same inherited indexes and existing overlay projection/cache/dirty storage. Named virtual adapters invoke the four already-recovered static algorithms; they receive no additional original-body credit. Only wrapper 1410fe3b0 is new. Cross-module class exports preserve normal module linkage.

Protected C++ support construction and nonvirtual destruction are not recovered lifecycle bodies and prevent ordinary unsupported allocation/deletion. Future original installation, destruction, writers and collectors must use these same classes and the same profile pointer. The omitted copy-group virtual is not replaced with a placeholder. Compiler layouts are not asserted to match the MSVC binary.

Results are borrowed from actual backend storage. Derived group queries may mutate cache/dirty state; item lookup retains the distinction between absent overrides and present null overrides. No null-backend guard, eager allocation, parallel owner, substitute map or fallback result is added. Newly constructed profiles retain the original null default and query callers retain the original installed-live-backend precondition.

The six hashes in native-evidence.json were independently checked against the cached original image. Source emission checks these windows and tables fail-closed. Inventory callers can be source-complete under the original precondition while profile installation/population/GC and product acceptance remain open. UE build/runtime pending for this batch.
