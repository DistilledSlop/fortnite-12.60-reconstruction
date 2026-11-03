void UFortUserWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (UFortGameInstance* Instance = Cast<UFortGameInstance>(GetGameInstance()))
        Instance->NativeRegisterClientEventObject141e78cb0(this);
}

void UFortUserWidget::NativeDestruct()
{
    if (UFortGameInstance* Instance = Cast<UFortGameInstance>(GetGameInstance()))
        Instance->NativeUnregisterClientEventObject141e84a70(this);
    Super::NativeDestruct();
}
