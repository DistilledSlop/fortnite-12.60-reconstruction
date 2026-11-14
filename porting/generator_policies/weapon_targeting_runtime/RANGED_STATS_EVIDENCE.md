# Ranged stats slot +0x6b0

`AFortWeaponRanged::NativeGetRangedWeaponStats6B0` is original `1426cf820`.
Ten constructor-proven ranged primary vtables select this body; thirteen sampled
non-ranged weapon classes select `140680300`, the exact zero-return body.
This slot differs from base-stat slot `+0x6a8` (`1426d0610` on ranged weapons).

The complete body rejects null WeaponData, returns the same borrowed override
row when bUseAthenaRecoil and that pointer are both nonzero, and otherwise
uses `FDataTableRowHandle::GetRow<FFortRangedWeaponStats>`. The override's actual
ranged-row identity remains the original producer precondition; no runtime type
guard or replacement row is introduced.

Direct standard captures in local-model-package/captures:

* `13935202af0e4ea795af901af1c16fca/0x1426cf820`: complete getter.
* `4d64634af6f2474e858efbc5f6b5f79d/0x1418de940`: validates row-struct derivation,
  Name_None and normal DataTable diagnostics.
* `fc56b8ef0d544bebb2334aa871721b19/0x142b51750`: registers FortRangedWeaponStats,
  size `0x2d0`, proving the actual typed lookup.

Pinned reflection has struct block `147c001a0`, constructor `142b51990`, parent
constructor `142b4b280` (FortBaseWeaponStats), and AutoReloadDelayOverride at
original `+0x2a0`. The normal generator type closure emits all reflected members
and inheritance from the helper signature. Linux consumers use the typed field,
never an original Windows byte offset. This source closure does not establish
the remaining targeting timer/reload action or runtime parity.
