# Inherited Athena pickup dispatch confirmation78

Root read six consecutive8-byte slots485..490 from the original runtime image
at primary table145e2cfd0, whose constructor provenance is documented in existing
binary_analysis.md. Slot/function names come from
porting/ghidra/name_tables/vtable_slots_AFortPlayerPawnAthena.json. evidence.json
records exact file offset, bytes, local SHA-256 and observed targets.

All three validators target1406801d0 and all three implementations1406801c0,
matching the canonical manifest's shared true/empty bodies. This closes the
specific concern that the inherited pickup RPCs might have different Athena
vtable targets. No additional body is claimed or counted. This does not show
where pickup gameplay is ultimately handled and is not a gameplay parity test.
The image was read only; no Ghidra edit or execution was needed.
