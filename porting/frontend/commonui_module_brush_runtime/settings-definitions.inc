// Candidate for the original constructor and 1407d0c30.
// Requires UObject/UObjectArray.h, UCommonUIRichTextData.h and Materials/MaterialInterface.h.
UCommonUISettings::UCommonUISettings()
    : bAutoLoadData(true), NativeDataLoaded(false)
{
    static_assert(STRUCT_OFFSET(UCommonUISettings, NativeDataLoaded) == 0xa8, "Original CommonUI loaded flag layout");
    static_assert(STRUCT_OFFSET(UCommonUISettings, DefaultThrobberBrush) == 0xc0, "Original CommonUI brush layout");
}

void UCommonUISettings::NativeInitializeData()
{
    if (NativeDataLoaded)
    {
        return;
    }

    DefaultImageResourceObjectInstance = DefaultImageResourceObject.LoadSynchronous();
    DefaultThrobberMaterialInstance = DefaultThrobberMaterial.LoadSynchronous();
    UClass* RichTextClass = DefaultRichTextDataClass.LoadSynchronous();
    RichTextDataInstance = RichTextClass ? static_cast<UCommonUIRichTextData*>(RichTextClass->GetDefaultObject()) : nullptr;

    if (GUObjectArray.IsDisregardForGC(this))
    {
        if (DefaultImageResourceObjectInstance) DefaultImageResourceObjectInstance->AddToRoot();
        if (DefaultThrobberMaterialInstance) DefaultThrobberMaterialInstance->AddToRoot();
        if (RichTextDataInstance) RichTextDataInstance->AddToRoot();
    }

    DefaultThrobberBrush.SetResourceObject(DefaultThrobberMaterialInstance);
    DefaultThrobberBrush.ImageSize = FVector2D(64.0f, 64.0f);
    DefaultThrobberBrush.ImageType = ESlateBrushImageType::FullColor;
    DefaultThrobberBrush.DrawAs = DefaultThrobberMaterialInstance
        ? ESlateBrushDrawType::Image : ESlateBrushDrawType::NoDrawType;
    NativeDataLoaded = true;
}
