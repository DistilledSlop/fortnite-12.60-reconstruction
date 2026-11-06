// CONFIDENCE: HIGH
// Original address: 0x142934e80
    UClass* LoadedClass = GiftBoxHeaderSubWidgetRef.LoadSynchronous();
    OutWidgetClass = LoadedClass;
    return LoadedClass && LoadedClass->IsChildOf(UUserWidget::StaticClass());
