# Last weapon or gadget selection: unresolved closure

No behavior snippet is proposed. The source bodies below have not been promoted,
compiled, or exercised. In particular, an empty or guessed equip operation would
lose both stack pruning and the actual equipment effects.

The authoritative input is `image-125.runtime.bin`, loaded at VA `0x140000000`,
SHA-256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`.
Raw entry bytes disassemble to the following control flow at `0x14234a320`:
call `0x142314d70` with the incoming `this` and `bForce`; if it returns false and
`bForce` is true, call the pawn virtual at `+0xbc0`. The captured
AFortPlayerPawnAthena vtable at `0x145e2cfd0` has `0x142314d40` in that slot;
other overrides must remain virtual. The decompiler omitted the helper arguments;
raw instructions preserve DL into DIL before the call and the helper preserves DL
for its final controller call.

Existing probe `generated/callee_probe/blocking/0x142314d70.{c,asm,json}` shows:

* First call `0x14230dbb0`, which walks `TempWeaponsStack` backwards and removes
  null definitions, and also removes definitions that the controller inventory
  interface cannot find when the controller is an AFortPlayerController. The
  TArray's shrink operation is `0x14072c6c0`; use of normal `RemoveAt` must retain
  equivalent shrink semantics. Reflection identifies `TempWeaponsStack` as an
  array at pawn `+0x2110` (count `+0x2118`).
* Cast pawn controller `+0x250` to AFortPlayerController. Iterate that stack in
  reverse; interface at controller `+0x598`, virtual `+0xa0`, resolves each
  definition with a false third argument. On the result, virtual `+0x468` yields
  a 16-byte GUID, and virtual `+0x3e8` yields a definition. Accept a valid GUID
  only when the definition is a `UFortWeaponItemDefinition` or
  `UFortGadgetItemDefinition`. The class getters at `0x142a3d9f0`,
  `0x142b4a5d0`, and `0x1428e9db0` were identified by their static class strings
  in `generated/callee_probe/weapon_selection/`.
* If no stack entry qualifies, test the nonzero 16 bytes at pawn `+0x1688`,
  resolve them via controller interface virtual `+0xb0`, and apply the same
  definition type check. This field's reflected/native name and producer remain
  unverified. On success call controller virtual `+0x1b10` with GUID, float zero,
  incoming `bForce`, true, and return true; otherwise return false. The argument
  shape resembles `ClientExecuteInventoryItem`, but its slot identity and full
  implementation must be established rather than assumed.

The captured forced fallback `0x142314d40` checks a pawn array at `+0x598` and,
if nonempty, calls `0x142314fe0` with its first pointer and returns true; otherwise
returns false. The 196-byte `0x142314fe0` body includes role-dependent work,
virtuals `+0xb0`, `+0x7b0`, `+0xc58`, `+0x6b0`, and calls `0x14269c8f0`,
`0x142a16c90`, and `0x142a19220`. Its actual contracts and mutations need full
reconstruction. The first exact missing implementation to investigate is
`0x142314fe0` and its three direct callees, alongside mapping its virtual targets;
independently identify the controller inventory interface's `+0xa0`/`+0xb0`,
item virtuals `+0x468`/`+0x3e8`, controller `+0x1b10`, and pawn `+0x1688`.
Until these are closed, neither this fallback nor the RPC is safe to splice.

Probe outputs are ignored artifacts under `generated/callee_probe/weapon_selection`,
`weapon_selection2`, and `weapon_selection3`. Commands used the existing
`probe_callees.py` with address lists at `/tmp/weapon-selection-probe{,2,3}.json`,
`--timeout 60 --mem 8G`. Ghidra's transaction rolls back; its project metadata
files changed on open. The raw captured windows have these SHA-256 values (VA,
body byte count, digest):

* `0x14234a320`, 53: `8e58e5a0d7e832b264e3367069ed6696d8f2d81e24fd1c1c68082f6d94ef4841`
* `0x142314d70`, 616: `f72e15d33681a799728abd3a930211a0d10824c263bf3e8d9703cb836e577edf`
* `0x14230dbb0`, 314: `e89c2441fc6650ab4837fa3c70dbeae1bce687399b1426008037ac30a07ae185`
* `0x142314d40`, 42: `cbc4f72c963527c61fd5582d56bdda7badcf0b8a48eb85c25717d920df4be43a`
* `0x142314fe0`, 196: `28f916b32325ec4206a1296341a15c749b591a9e4d05f4613f03f57b9f22aa74`

These are capture-window identifiers, not a full proof that every Ghidra probe
instruction matches the authoritative capture. Verify each body before porting.
