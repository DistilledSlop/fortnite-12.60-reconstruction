// WorldItem virtual override 0x14205b6b0.
// The owner lookup is virtual: other interface implementers may override it.
IFortInventoryOwnerInterface* InventoryOwner = nullptr;
bool bViewTargetInventory = false;
AFortPlayerController* OwningController = GetOwningController();
if (OwningController)
{
    InventoryOwner = static_cast<IFortInventoryOwnerInterface*>(OwningController);
}
else
{
    if (NativeGetOwnerInventory())
    {
        AFortInventory* Inventory = NativeGetOwnerInventory();
        AActor* InventoryActorOwner = Inventory->GetOwner();
        if (InventoryActorOwner)
        {
            InventoryOwner = Cast<IFortInventoryOwnerInterface>(InventoryActorOwner);
        }
    }
    if (!InventoryOwner)
    {
        UWorld* World = GetWorld();
        if (!World)
        {
            return -1;
        }
        APlayerController* LocalController = GEngine->GetFirstLocalPlayerController(World);
        if (LocalController)
        {
            AFortPlayerController* FortController = Cast<AFortPlayerController>(LocalController);
            if (FortController && FortController->ViewTargetInventory == NativeGetOwnerInventory())
            {
                bViewTargetInventory = true;
                InventoryOwner = static_cast<IFortInventoryOwnerInterface*>(FortController);
            }
            AFortPlayerControllerFrontEnd* FrontEndController = Cast<AFortPlayerControllerFrontEnd>(LocalController);
            if (FrontEndController)
            {
                InventoryOwner = static_cast<IFortInventoryOwnerInterface*>(FrontEndController);
            }
        }
        if (!InventoryOwner)
        {
            return -1;
        }
    }
}
UFortWorldItemDefinition* WorldDefinition = Cast<UFortWorldItemDefinition>(GetItemDefinitionBP());
if (UFortWeaponItemDefinition* WeaponDefinition = Cast<UFortWeaponItemDefinition>(WorldDefinition))
{
    UFortAmmoItemDefinition* AmmoDefinition = Cast<UFortAmmoItemDefinition>(WeaponDefinition->GetAmmoWorldItemDefinitionNative());
    if (AmmoDefinition && *reinterpret_cast<const uint8*>(&AmmoDefinition->bIsConsumed) == 0)
    {
        return MAX_int32;
    }
    if (InventoryOwner->NativeHasInfiniteAmmo(WeaponDefinition) && !bViewTargetInventory)
    {
        return MAX_int32;
    }
}
UFortWorldItemDefinition* AmmoDefinition = WorldDefinition->GetAmmoWorldItemDefinitionNative();
if (!AmmoDefinition || AmmoDefinition == WorldDefinition)
{
    return -1;
}
UFortItem* AmmoItem = InventoryOwner->NativeFindItemByDefinition(AmmoDefinition, false);
const int32 Count = AmmoItem ? AmmoItem->GetNumInStack() : 0;
const uint32 Total = static_cast<uint32>(Count) + static_cast<uint32>(GetLoadedAmmo());
int32 Result;
FMemory::Memcpy(&Result, &Total, sizeof(Result));
return Result;
