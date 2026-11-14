// CONFIDENCE: HIGH
// Original 0x141946e10.
// AFortPlayerPawn::NativeGetInventoryOwnerInterfaceBD8, 0x141946e10
AController* OwnerController = GetController();
if (!OwnerController)
{
    return nullptr;
}
if (AFortPlayerController* PlayerController = Cast<AFortPlayerController>(OwnerController))
{
    return static_cast<IFortInventoryOwnerInterface*>(PlayerController);
}
if (AFortAthenaAIBotController* BotController = Cast<AFortAthenaAIBotController>(OwnerController))
{
    return static_cast<IFortInventoryOwnerInterface*>(BotController);
}
return static_cast<IFortInventoryOwnerInterface*>(
    OwnerController->GetNativeInterfaceAddress(UFortInventoryOwnerInterface::StaticClass()));
