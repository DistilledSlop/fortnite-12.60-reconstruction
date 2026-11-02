// 145174fa0. The original queues a weak reference and marks pending kill.
// It does not substitute UWidget::RemoveFromParent for that lifetime contract.
void NativeRemoveWidget145174fa0(UUserWidget* Widget)
{
    if (!Widget)
        return;
    UE_LOG(LogFortUI, Verbose, TEXT("UI Director is trashing widget [%s]"), *Widget->GetFName().ToString());
    if (UWorld* World = Widget->GetWorld())
    {
        if (UGameInstance* Instance = World->GetGameInstance())
        {
            if (UFortUIDirector* Director = Instance->GetSubsystem<UFortUIDirector>())
                Director->NativePendingRemovalWidgetsB8.Add(Widget);
        }
    }
    Widget->MarkPendingKill();
}
