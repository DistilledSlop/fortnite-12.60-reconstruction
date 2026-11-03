# HideToast TeamMember (+0x290) class: vtable to class mapping

Evidence: image = /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin (file offset = VA - 0x140000000),
read with objdump -D -b binary and byte scans. Decompiles are not used as evidence. Class names come from UTF-16 strings in the image.

## Result
- vtable 0x1465318e0 = UFortTeamMemberCampaign (verified below). Its slot 0x268 = 0x142292160, the implementation the wrapper calls.
- vtable 0x146531640 = UFortTeamMember, the base class (verified below). Its slot 0x268 = 0x1422920c0.
- The runtime class of the object stored at HideToast this+0x290 is UNKNOWN (see Open items).

## Class to constructor (verified)
| Class | Noregister fn | Name string | InternalConstructor passed | Ctor body / vptr store |
|---|---|---|---|---|
| UFortTeamMember (block 0x14652f830) | 0x142aef720 | "UFortTeamMember" at 0x146530c58 (asm 0x142aef75d lea 0x146530c5a = "FortTeamMember") | 0x142af8560 (lea 0x142aef785, stored to -0x40(%r11) at 0x142aef78c) | 0x142af8560: `jmp 0x142afa270` (asm 0x142af856e). 0x142afa270 stores vtable 0x146531640 at 0x142afa285 |
| UFortTeamMemberCampaign (block 0x14652f900) | 0x142aef8a0 | "UFortTeamMemberCampaign" at 0x146530cf0 (asm lea 0x146530cf2 at 0x142aef8dd) | 0x142af85c0 (lea at 0x142aef905, stored to -0x40(%r11) at 0x142aef90c) | 0x142af85c0: `call 0x142afa270` (0x142af85d4), then vptr store of 0x1465318e0 at 0x142af85e0 |
| UFortTeamMemberAthena (block 0x14652f960) | 0x142aefa90 | "UFortTeamMemberAthena" at 0x146530d60 | 0x142af8580 (lea at 0x142aefaf5) | 0x142af8580: `call 0x142afa270` (0x142af8594), then vptr store of 0x146531b80 at 0x142af85a0 |

- Campaign and Athena noregister functions both reference 0x142aef720 (UFortTeamMember noregister) as a lea operand (0x142aef8e4, 0x142aefad4). This is consistent with a base-class link, but the super argument slot is not separately checked.
- Block-to-noregister links come from fortnite_game_reflection.json (classes_in_groups_with_fort_names, class_noregister_fn). The name strings were read from the image.

## Vtable slot 0x268 (verified from image data)
- 0x146531640 (UFortTeamMember): slot 0x268 = 0x1422920c0
- 0x1465318e0 (UFortTeamMemberCampaign): slot 0x268 = 0x142292160 (the wrapper's callee, wrapper 0x142292260 at 0x142292290-0x142292293)
- 0x146531b80 (UFortTeamMemberAthena): slot 0x268 = 0x1422920c0
- Only the Campaign vtable maps slot 0x268 to 0x142292160 among these three.

## Shared base (verified)
- Campaign vtable differs from base in 117 of the first 290 slots (including slot 0x0, 0x260, 0x268, 0x270, 0x278, 0x280, 0x288, 0x290, 0x2a0). Athena differs in 183.
- Campaign and Athena both call base ctor 0x142afa270 before storing their own vptr. Base is UFortTeamMember.

## Corrections to earlier notes
- porting/generated/reflection/native_class_index.json lists vtable_candidate 0x146531640 for both UFortTeamMemberCampaign and UFortTeamMemberAthena. Neither matches: Campaign stores 0x1465318e0 and Athena stores 0x146531b80. 0x146531640 is the base UFortTeamMember vtable.
- NOTES.md item 2 is answered only for the vtable mapping above. The owner of the multicast at TeamMember+0x78 is still UNKNOWN.

## Open items (UNKNOWN)
1. Runtime class of the object at toast this+0x290: no setter found. The toast's SetToast decompile (decomp-imports/Imports/SetToast__144d97670.c) is the friend toast, has no +0x290 store, and is not the power-rating toast. Native bodies of ShowToast (create func 0x1452a13d0) and SetToast (0x1452a1370) are not recorded in the reflection, so their +0x290 stores were not scanned. A scan of the 596 `mov r64,[reg+0x290]` encodings is not attributed to the toast.
2. Whether the object at +0x290 is constructed by 0x142af85c0 (Campaign) at runtime: UNKNOWN. HANDOFF.md's claim that 0x142af85c0 initializes the weak object at +0x90 is not verified here.
3. Wrapper 0x142292260 calls slot 0x268 on TeamMember; which vtable that object carries at runtime is UNKNOWN, so the slot-0x268 implementation for this object is UNKNOWN beyond the static mapping above.

## VERIFIED: setter of the weak team member (toast +0x290) and its class
Evidence: porting/ghidra/find_field_refs.py (LEA of [reg+0x290] followed by a weak-pointer helper call, window 0x300000 around HideToast) found the only candidates; the reflected exec of UFortPlayerPowerRatingToast::SetToast is 0x1452a5350 (native_funcs.json), which calls the body 0x144e1cd20(this, notification).
- 0x144e1cd20 assigns the weak pointer with `FUN_142ee3b30(this+0x290, object)` (LEA at 0x144e1ceea) only when the object passes a class check against `FUN_142aef8a0()`: object->Class->inheritance array [depth at class+0x38] == class+0x30, the fast IsA pattern.
- 0x142aef8a0 is the UFortTeamMemberCampaign no-register (class-getter) function (verified earlier in this file: its string is the class name).
- Therefore the weak slot at toast+0x290 holds a UFortTeamMemberCampaign (assigned only after IsA(UFortTeamMemberCampaign)). The same body also stores the object into the sub-widget at this+0x270 via 0x144e1c1a0 before assigning.
- The object is obtained from the notification payload: 0x142195490(world-derived object, notification+0x60) then the IsA check (the first stage 0x141f5dd20/0x141e63650 chain is not decoded).
Still UNKNOWN: the three multicast delegate types (the listener-adding calls are in 0x144e22b70, the function called last by 0x144e1cd20; not yet decoded).
