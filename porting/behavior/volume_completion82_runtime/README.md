# Volume loading/unloading completion RPCs

Original1419f19c0 and1419f1b80 scan their reflected VolumesLoading (+0x35f8)
and VolumesUnloading (+0x3608) arrays for the supplied pointer. If found they call
140726fc0 and return; if absent they log at verbosity3. Probe82 recovered helper
140726fc0: it walks alternating equal/unequal runs, memmoves survivor runs,
updates ArrayNum, returns removed count, and never reallocates or shrinks.
TArray::Remove on pointer elements performs that stable remove-all operation;
Contains preserves the original initial search and absent-item branch.

The assembly shows non-null objects load FName at +0x18 and call142ce9f90,
FName::ToString. Null uses literal None. Original log category1481348e0/e4 is
LogFort, established in ../aircraft_lookup_evidence.md. Both warning format
strings deliberately retain "not currently loading", including unloading.
UE_LOG evaluates the string argument only when Warning is enabled. Required
headers expose AFortVolume inheritance and the existing shared category.

Sources and hashes are in sources.json. Root ran a single rolled-back Ghidra
probe for remove helper, both RPCs and handshake setter. Probe outputs under
porting/generated/volume-integration82, log /tmp/volume-integration82.log.
No compiler, build, runtime or tests. Acceptance depends on root generation.

Root generation82 accepted both RPCs,2,276 total entries/0 rejected after the
full19-body batch. No build/runtime validation; all19 are build pending.
