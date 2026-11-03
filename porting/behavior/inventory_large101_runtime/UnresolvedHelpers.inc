// UNCOUNTED / UNCOMPILED. Remaining unresolved natives after native101-inventory-deps. Each is declared with the
// exact call shape seen in the proven bodies; NO semantics are assumed. Resolved helpers are lifted inline in the
// two .inc bodies (142bedc40, 1407c2c80, 142c165b0, 142c40560, 14129b8c0, 1450b2c80, 14204e530, 144cf0e90,
// 14447f220, 144d07980, 142074570, 142057010, 1423881a0, 14511b280-shape).
namespace InventoryLarge101Native
{
// ---- CanItemBeMulched 0x144cfad30 ----
// NOT in native101 deps: reason-code producer for non-WorldItems (writes TArray<uint8>, returns bool). Needs body.
bool Native_144cfb480(const UFortInventoryContext* Ctx, UFortItem* Item, TArray<uint8>& OutCodes);
// Non-WorldItem (UFortAccountItem path) virtual +0x4f0 -> bool. UFortItem/WorldItem table targets: Item=0x1406801e0 (returns 0);
// WorldItem=0x142058100 (calls this+vfunc 0x4e0(0,1) then tail-jumps with this+0xe0). UFortAccountItem table NOT constructor-proven: need it.
bool Item_Slot0x4f0(UFortItem* Item);
// FUN_140822780(Ctx, UClass*): call used by 144cf0e90 (typed-outer-lookup shape; body not provided). Class arg = UFortHomebaseUIContext (getter 0x1452518e0).
UObject* Native_140822780(const UObject* Ctx, UClass* Class);
// Hero lookup key/hash helpers used by 14511b280 (bodies not provided).
void Native_142ee3b30(int32 OutKey[2], UFortItem* Item);
void Native_141656200(const void* Map, int32* OutIndex, const int32 Key[2]); // -1 when absent; entries stride 0x1c, value at entry+8
// FDataTableRowHandle row fetch used by 14204e530 (body not provided). Non-null result == true.
void* Native_141ff8510(const FDataTableRowHandle& Handle, const FString& ContextString);

// ---- DropItemOnQuickBar 0x144cfe010 ----
// Inline object at AFortPlayerController+0x598 (NOT reflected; has its own vtable). Constructor/table not proven.
AFortQuickBars* Native_PC598_Slot0x50(AFortPlayerController* PC);   // virtual +0x50, returns quickbars (class proven via 142057010: offsets 0x220/0x2b0/0x340)
bool Native_PC598_Slot0x20(AFortPlayerController* PC);              // virtual +0x20, bool
// AFortQuickBars helpers whose bodies are not available / too large.
int32 Native_1420570e0(FQuickBar* Bar, const FGuid& ItemGuid);      // slot index or -1 (used by 142057010)
bool Native_142070440(AFortQuickBars* QB, uint8 ExistingBar, int32 ExistingSlot, uint8 TargetBar, int32 TargetSlot); // 569-line body, not lifted here
bool QB_Slot0x628(AFortQuickBars* QB, UFortItem* Item, uint8 Bar, int32 Slot, uint8 Flag);
UFortItem* QB_Slot0x648(AFortQuickBars* QB, uint8 Bar, int32 Slot);
// AFortPlayerController virtuals/body not available.
bool PC_Slot0x15f0(AFortPlayerController* PC, const void* Arg);     // arg = Native_141ed1800() + 0x6c0
const uint8* Native_141ed1800();
UFortItem* PC_Slot0x1a80(AFortPlayerController* PC, uint8 Bar, int32 Index); // used by 144d07980
// Notification on PC with FName{0x8e1,0} (global 0x14838a438). 438-byte body 0x1423bd380 read but effect chain (143f7a920/142a3ca80/1424e1120/144230ed0) unresolved.
void Native_1423bd380(AFortPlayerController* PC);
// RPC-by-name via UObject::FindFunctionChecked(FName idx 0x4fcbb at global 0x1481af118) + ProcessEvent (vtbl +0x210), params {A(16B), B(16B)}.
// Signature-compatible with AFortPlayerController::ServerCombineInventoryItems(SourceItemGuid, TargetItemGuid) but FName text NOT decoded (need FNamePool).
void Native_142a3e840(AFortPlayerController* PC, const FGuid& First, const FGuid& Second);
// ProcessEvent on Receiver (= MyPlayerInfo+0xf0) with FName idx 0x48ad9 (global 0x1481a32f0): params {TArray<FString> copy, FString copy,
// struct copy of 0x1480e9bb0 (zero in image), local FString "McpClientCommandUrl", ...}. Function name not decoded.
void Native_1429b0bb0(void* Receiver, const TArray<FString>& A, const FString& B, const void* C_1480e9bb0);
}
// Additional raw item virtuals used by DropItemOnQuickBar (outside the namespace above for brevity).
namespace InventoryLarge101Native
{
int32 Item_Slot0x368(UFortItem* Item);   // base 0x140833bf0 returns 1; WorldItem 0x14205ac70 -> ItemEntry (this+0xe0) helper
FString Item_Slot0x470(UFortItem* Item); // WorldItem 0x14205a1a0: checks byte this+0xd0, calls vtbl +0x468 (guid) then a guid->string helper; returns FString via hidden ptr
}
