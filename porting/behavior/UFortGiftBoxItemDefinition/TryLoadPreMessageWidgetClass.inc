// CONFIDENCE: HIGH
// Original address: 0x142934f80
    UClass* LoadedClass = GiftBoxPreMessageWidgetRef.LoadSynchronous();
    OutWidgetClass = LoadedClass;
    return LoadedClass && LoadedClass->IsChildOf(UUserWidget::StaticClass());
