void UCommonInputSubsystem::NativeBroadcastInputMethodChanged()
{
    UWorld* World = GetWorld();
    if (World && !World->bIsTearingDown)
    {
        NativeInputMethodChanged.Broadcast(CurrentInputType);
        OnInputMethodChanged.Broadcast(CurrentInputType);
        LastInputMethodChangeTime = FPlatformTime::Seconds();
    }
}
