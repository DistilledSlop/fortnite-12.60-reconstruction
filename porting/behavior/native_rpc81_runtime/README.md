# Two native RPC integrations, batch81

`ServerSetPlayset_Implementation` assigns the mapped `CurrentPlayset` pointer.
`ServerUnmarkRespawned_Implementation` clears the mapped `bIsRespawning` and
`bIsRespawningInAir` flags in observed order. Both signatures and fields already
exist in the generated classes. `evidence.json` pins complete original native
instruction windows through RET and generated decompile sources. Original bytes
were read by VA minus image base and reviewed with GNU objdump; no Capstone was
available in either system Python or the project venv.

The pointer body is `mov [rcx+0x3588],rdx; ret`. The flag body is
`and byte [rcx+0x10e9],0x7f; and byte [rcx+0x10ea],0xfe; ret`.
`porting/generated/reflection/bool_bits.json` maps those two offset/mask pairs to
these reflected bitfields; `pawn_state_evidence.md` independently records the
same mapping. `CurrentPlayset` is already used by the accepted GetPlayset getter.

Canonical generation81 accepted2,257 entries (2,077 bodies plus180 fields),
rejected0, and changed only the two expected native implementation files. No
build/runtime/tests ran: last tested2,040, next checkpoint3,040. These two bodies
are source-integrated and build pending, without a runtime parity claim.
