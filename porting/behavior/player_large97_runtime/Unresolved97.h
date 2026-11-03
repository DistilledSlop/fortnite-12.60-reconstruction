// UNCOUNTED DRAFT - not compiled, not promoted. Declarations only: every function here is an effect whose body has NOT been
// recovered. Nothing below may be given a guessed body; callers keep the native call so the dependency stays visible.
#pragma once
#include "CoreMinimal.h"
#include "EDeathCause.h"

// ---- raw layout views (offsets proven from the decompiles; reflected types are used wherever the generated headers carry them) ----
struct FRawTag { int32 ComparisonIndex; int32 Number; };                 // FGameplayTag as the native compares it (two int32)
struct FRawTagArray { FRawTag* Data; int32 Num; int32 Max; };             // TArray<FGameplayTag> view, 0x10 bytes
struct FRawTagContainerView { FRawTagArray GameplayTags; FRawTagArray ParentTags; }; // FGameplayTagContainer: native reads qword[0],int[2] and qword[2],int[6]
static_assert(sizeof(FRawTagContainerView) == 0x20, "FGameplayTagContainer view");

enum class EFortToDeathCauseRow : uint8 { Fixed, SelectByDBNO, NestedSniper, LastRow };
struct FFortToDeathCauseRow { uint32 TagSlotOffset; EFortToDeathCauseRow Kind; EDeathCause ResultA; EDeathCause ResultB; };

struct FRawInterfaceRef { UObject* Object; void* Interface; };            // 16-byte element of the array at controller+0x2da0

// ---- ToDeathCause (0x141a5f1c0) ----
const uint8* Unresolved_FFortGameplayTags_Singleton_141ed1800();         // returns a table of FGameplayTag slots addressed by byte offset (+0x1628, ...); slot->tag-name mapping not recovered
UObject*     Unresolved_DeathCauseOverrideProvider_141e5b2a0();
void         Unresolved_DeathCauseOverrideLookup_141e5f200(UObject* Provider, const struct FRawTagContainerView* Tags, bool bFlag, uint8* InOutCause); // may overwrite *InOutCause (native pre-seeds 0x32)
bool         Unresolved_TagArrayHasTag_14192b2d0(const FRawTagArray& Array, const FRawTag& Tag); // called with &Container[0] then &Container[0x10]

// ---- AFortQuickBars::SwapItems (0x142070440) and CheckGhostModeItemReplicated (0x1423cbe80) ----
// Argument lists below are taken from the x86 call sites (stack args the decompiler dropped are included).
struct FRawSlotMove { UObject* Item; int32 SlotIndex; int32 Pad; };                          // 16-byte entries of the push list
struct FRawPendingMoveEntry { FGuid ItemGuid; UObject* Item; int32 OldSlot; int32 NewSlot; int32 HashNext; int32 HashIndex; }; // 0x28 TSet element (shape inferred from the drain loop only)
struct FRawPendingMoveSet;                                                                    // TSet/TSparseArray + inline bit array (stack object at rbp+0x50); layout not recovered
bool   Unresolved_QuickBars_CanFitItem_14204e850(class AFortQuickBars* QB, UObject* Item, struct FQuickBar* Bar, int32 SlotIndex, const TArray<FGuid>& IgnoredGuids, bool bFlag /*=false at both call sites*/);
bool   Unresolved_QuickBars_TryFitWithPush_14204f2a0(class AFortQuickBars* QB, TArray<FRawSlotMove>& Moves, struct FQuickBar* Bar, bool bPushDirection, const TArray<FGuid>& IgnoredGuids, FRawPendingMoveSet& OutPending);
void   Unresolved_QuickBar_RemoveItem_142065b00(struct FQuickBar* Bar, const FGuid& ItemGuid, int32 SlotIndex);
void   Unresolved_QuickBar_PlaceItem_142049220(struct FQuickBar* Bar, UObject* Item, int32 SlotIndex);
void   Unresolved_QuickBars_NotifyItemMoved_1417cdda0(UObject* Controller, UObject* Item, uint8 Bar, int32 IdxX, int32 IdxY, UObject* OtherItem);
void   Unresolved_QuickBars_FocusChanged_14204dd50(void* QuickBarsPlusBytes0x400, uint8 BarType, int32 SlotIndex);
void   Unresolved_QuickBars_RefreshBar_14205c5e0(class AFortQuickBars* QB, uint8 BarType);
bool   Unresolved_QuickBars_FindItemSlot_142057010(class AFortQuickBars* QB, UObject* Item, uint8* OutBarType, int32* OutSlot); // decompiled in native96-impl: IsA check, item virtual +0x468 -> FGuid, scans bars 0..2 via 0x1420570e0 (unresolved)
UObject* Unresolved_QuickBars_GetFocusedItem_142059e40(class AFortQuickBars* QB);   // first bar (0,1,2) with CurrentFocusedSlot != -1, then tail-calls QuickBars virtual +0x648
UClass*  Unresolved_StaticClass_142a3d9f0();   // class used by the Cast on QuickBars Owner
UClass*  Unresolved_StaticClass_142b5be40();   // class used by the Cast on GhostModeRepData.GhostModeItemDef
UObject* Unresolved_InventoryOwnerInterface_FindItemByGuid_vtB0(void* InterfaceAt0x598, const FGuid& Guid); // interface at controller+0x598, vtable slot +0xb0
uint8  Unresolved_Item_GetItemSize_vt2e8(UObject* Item);                         // virtual +0x2e8, returns byte
uint8  Unresolved_Item_GetItemType_vt318(UObject* Item);                         // virtual +0x318, returns byte compared with AcceptedItemTypes entries
extern uint8 Unresolved_LogVerbosity_1481348e0;                                   // category runtime verbosity; native logs only when >= 2 (Error)
void   Unresolved_LogSwapItemsError_142c54b70(const TCHAR* Format, int32 IndexA, int32 IndexB, int32 Arg3, int32 Arg4); // FMsg::Logf_Internal(nullptr,0,&Category@0x1481348e4,2,Fmt,...)
// CheckGhostModeItemReplicated-only virtuals (interface at controller+0x598 unless noted)
class AFortQuickBars* Unresolved_InventoryOwnerInterface_GetQuickBars_vt50(void* InterfaceAt0x598);   // vtable +0x50
void   Unresolved_InventoryOwnerInterface_GetItemsOfType_vtD0(void* InterfaceAt0x598, uint8 ItemType, TArray<UObject*>& Out); // vtable +0xd0, arg dl=0x17 (EFortItemType::WeaponHarvest)
void   Unresolved_QuickBars_vt630(class AFortQuickBars* QB, UObject* Item, int32 Arg0, bool bArg1); // r9b=1  // virtual +0x630 (called with (item,0,true) for the harvest tool)
void   Unresolved_QuickBars_vt628(class AFortQuickBars* QB, UObject* Item, int32 Arg0, int32 Arg1, bool bArg2); // virtual +0x628
void   Unresolved_QuickBars_vt640(class AFortQuickBars* QB, int32 A, int32 B, float C, bool D, bool E);   // virtual +0x640 (called (0,0,0.0f,true,true): edx,r8d,xmm3,[rsp+20],[rsp+28])
const uint32* Unresolved_Item_vt480(UObject* Item, void* OutTemp);          // item virtual +0x480 -> pointer; first dword is the key fed to 0x140708a10
UObject* Unresolved_InventoryOwnerInterface_FindItemByDefinition_vtA0(void* InterfaceAt0x598, UObject* ItemDef); // vtable +0xa0 (rdx = GhostModeItemDef cast, may be null)
UObject* Unresolved_Controller_vt14d8(UObject* Controller);                    // controller virtual +0x14d8 (object whose +0x4c8 member has vtable slot +0x10)
UObject* Unresolved_SubObject4c8_vt10(void* SubObjectAt0x4c8);
UObject* Unresolved_LookupByDword_140708a10(UObject* Container, uint32 Key);
// timer
struct FTimerManager* Unresolved_World_GetTimerManager_raw(UWorld* World);         // native: GameInstance(+0x188)? ->TimerManager(+0xc0) : World+0x320
void   Unresolved_TimerManager_SetTimer_1446278d0(FTimerManager* TM, uint64* InOutHandle, void* Delegate, float Rate, bool bLoop, float FirstDelay);

// ---- CheckIfLevelStreamingCompleteForHandshake (0x1419dbc20) ----
extern int32 Unresolved_Global_14811365c;
struct FRawObjectIterState;
void     Unresolved_ObjectIterCtor_140894360(FRawObjectIterState* It, UWorld* World, UClass* Class);
void     Unresolved_ObjectIterDtor_14469a960(FRawObjectIterState* It);
UObject* Unresolved_ObjectItemResolveActor_143f76730(UObject* Item);
UWorld*  Unresolved_ActorResolveIterWorld_1413064f0(UObject* Actor);
UClass*  Unresolved_StaticClass_14273ec00();
UClass*  Unresolved_StaticClass_14480eb00();
bool     Unresolved_IterStub_ReturnFalse();
UObject* Unresolved_ActorStateRef_14468e190();
const struct FBox* Unresolved_ActorStreamingBounds_141bdf240(UObject* Actor);   // FBox{Min,Max}; only X/Y ranges are read
void   Unresolved_DrainPendingMoves_Bar(struct FQuickBar* BarA, FRawPendingMoveSet& Pending); // native inline loop: RemoveItem(BarA,&elem.guid,elem.oldSlot) then PlaceItem(BarA,elem.item,elem.newSlot) per set bit
// level-streaming object virtuals/fields used by 0x1419dbc20 (ULevelStreaming-like objects; names not recovered, ECurrentState byte at +0xb8, loaded-level pointer at +0x128)
bool   Unresolved_LevelStreaming_vt270(UObject* L);
bool   Unresolved_LevelStreaming_vt268(UObject* L);
bool   Unresolved_Listener_vt28(UObject* L);                       // virtual +0x28 on the object at controller+0x2da0[i].Object
bool   Unresolved_Listener_vt48(UObject* L, UObject* Controller);  // virtual +0x48(this, controller)
