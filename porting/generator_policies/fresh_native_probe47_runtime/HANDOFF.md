# Fresh native probe 47

25 AFortWeapon/AFortWeaponRanged native callees were probed against original-late-capture-1, image base 0x140000000, via the read-only Ghidra transaction. `native-evidence.json` contains whole returned Ghidra callee decompile and assembly windows, including metadata; `wrapper-evidence.json` contains reflected records and wrapper disassembly (through padding, at most 512 bytes). `/tmp/fresh-native47/probe.log` records the probe. The Ghidra lease was released on completion.

Three closed high-confidence bodies and manifest entries: DoesHaveOverrideWrap, GetCoreAnimation, GetWeaponPawnAnimSet. Reflection block 0x14656a020 identifies the fields used: OverrideItemWrapSoftPtr +0x3e0, WeaponCoreAnimation +0xab0, WeaponPawnAnimSet +0xab8, WeaponPawnAnimsetOverride +0xac8. The native null check is the soft path's FName at +0x3f0; UE's TSoftObjectPtr::IsNull / FSoftObjectPath::IsNull / FName::IsNone match the two zero-word checks. These are source proposals; no builds or runtime validation were performed.

Remaining 22 require further evidence, not blanket replacement with approximate behavior:

- GetBulletsPerClip (0x14269b000): virtual +0x6a8, owner/class checks and scalable stat machinery; its early +0x33e/+0x344 override is only one path.
- GetChargePercent and GetChargeToMinPercent (0x14269b900/0x14269b9a0): virtuals +0x6a8/+0x150 and helpers 0x1426786a0, 0x142695260 establish charge limits.
- GetCurrentMaxChargePercent (0x14269ba70): decompiler misses most body; assembly shows virtual +0x6a8, helper 0x142668ff0, integer +0x8d4 and clamped ratio.
- GetMeleeAttackPlayRate (0x14269d6c0): class checks, stat scaling and owner-dependent helpers.
- GetWeaponData (0x1426a0960): field +0x350 plus conditional invalid-data logging; GetWeaponDataAmmoWorldItemDefinition (0x1426a09e0) adds helper 0x1420589d0 and logging. Preserve logging side effects unless proven dispensable.
- GetWeaponDataItemType and GetWeaponDataRarity (0x1426a0b10/0x1426a0c40): WeaponData virtual slots +0x298/+0x2a8, fallback enum values 0x70/7 and logging.
- GetWeaponDataTriggerType (0x1426a0ec0): helper 0x142671d20 and invalid-data logging.
- GetWeaponRateOfFireModifier (0x1426a16b0): helper 0x142692a20 with 1.0 input and return ABI.
- HasInfiniteAmmo (0x1426a24a0) and InventoryOwnerHasInfiniteAmmo (0x1426a58f0): helper 0x14269c8f0, interface virtual +0x60, WeaponData and weapon virtual +0x820; exact interface contract needed.
- IsSuppressed (0x1426a5db0): Ghidra reports unreachable blocks and a suspect constant return; requires disassembly and full helper resolution before any body.
- DoesWeaponOverheat (0x1426c8610): helper 0x1426ce850 > 0.0; helper identity/body unresolved.
- GetADSTransitionInTime and GetADSTransitionOutTime (0x1426c9f70/0x1426ca040): owner helper 0x14269e3e0, thunk 0x141ed8de0, virtuals +0x6a0/+0x6b0 and class checks.
- GetOverheatingMaxValue (0x1426ced60): owner/ability-system stat scaling, virtual +0x6b0 and helper 0x1426cbcd0.
- IsOverheated (0x1426d2560): mutates state depending on owner/world time, virtual +0x150, time slot +0x630 and helper 0x1426cea10; decompiler signed-byte tests require assembly review.
- IsUsingFirstPersonCamera (0x1426d28d0): helper 0x143f76650, class test, virtual +0x6a8 and helper 0x1423908a0.
- IsUsingScope and IsUsingScopeOrFirstPersonCamera (0x1426d2930/0x1426d29b0): owner cast, camera helper 0x142321a70, reflected/state checks and first-person dependencies.

No canonical source files, commits, build, tests, or runtime were changed or run in this batch. Original decompiler sometimes truncates or misinterprets float return and control flow; consult the assembly windows.

Integration follow-up: GetWeaponData, GetWeaponDataAmmoWorldItemDefinition and
GetWeaponDataItemType are now also integrated from remainder-manifest-entries.json.
Their warning strings and definition virtual dispatch are preserved. The original
survey bullets above record earlier findings; these three are no longer blockers.
No identity is assigned to weapon virtual +0x6a8. The reflected GetWeaponData body
is called directly and must not replace that virtual without separate evidence.
Root normalized native-evidence.json to the shared raw-window validation schema;
probe-evidence.json retains the detailed original 25-callee decompilation bundle.
