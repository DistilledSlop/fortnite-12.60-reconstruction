// Candidate for the original constructor and 1407d0c30.
// Requires UObject/UObjectArray.h, UCommonUIRichTextData.h and Materials/MaterialInterface.h.
UCommonUISettings::UCommonUISettings()
    : bAutoLoadData(true), NativeDataLoaded(false)
{
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
    RichTextDataInstance = RichTextClass ? RichTextClass->GetDefaultObject<UCommonUIRichTextData>() : nullptr;

    if (GUObjectArray.IsDisregardForGC(this))
    {
        if (DefaultImageResourceObjectInstance) DefaultImageResourceObjectInstance->AddToRoot();
        if (DefaultThrobberMaterialInstance) DefaultThrobberMaterialInstance->AddToRoot();
        if (RichTextDataInstance) RichTextDataInstance->AddToRoot();
    }

    DefaultThrobberBrush.SetResourceObject(DefaultThrobberMaterialInstance);
    DefaultThrobberBrush.ImageSize = FVector2D(64.0f, 64.0f);
    DefaultThrobberBrush.DrawAs = DefaultThrobberMaterialInstance
        ? ESlateBrushDrawType::Image : ESlateBrushDrawType::NoDrawType;
    DefaultThrobberBrush.ImageType = ESlateBrushImageType::FullColor;
    NativeDataLoaded = true;
}
