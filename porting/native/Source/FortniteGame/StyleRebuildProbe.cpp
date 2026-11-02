#include "StyleRebuildProbe.h"
#include "UCommonButtonInternal.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundWave.h"
#include "Engine/Texture2D.h"
#include "Widgets/SWidget.h"
#include "UObject/UnrealType.h"
#include "UObject/StrongObjectPtr.h"

struct FCommonButtonStyleRebuildProbeAccess
{
    static void Bind(UCommonButton& Button, UCommonButtonInternal* Internal) { Button.NativeInternalButton = Internal; }
    static void State(UCommonButton& Button, bool Selected, bool Enabled) { Button.bNativeSelected = Selected; Button.bNativeButtonEnabled = Enabled; }
    static void Rebuild(UCommonButton& Button) { Button.NativeRebuildStyle(); }
    static void Refresh(UCommonButton& Button) { Button.NativeRefreshCurrentStyle(); }
    static UCommonButtonStyle* CDO(UCommonButton& Button) { return Button.NativeGetStyleCDO(); }
};

void URebuildProbeButton::NativeOnCurrentTextStyleChanged()
{
    ++NativeEvents;
    Super::NativeOnCurrentTextStyleChanged();
}

void URebuildProbeButton::ProcessEvent(UFunction* Function, void* Params)
{
    Super::ProcessEvent(Function, Params);
    if (Function && Function->GetFName() == FName(TEXT("OnCurrentTextStyleChanged")))
    {
        ++ReflectedEvents;
        const TSubclassOf<UCommonTextStyle> Current = GetCurrentTextStyleClass();
        if (Current) ObservedFontSize = Current.GetDefaultObject()->Font.Size;
        if (bClearBrushColorsAtEvent)
        {
            bClearBrushColorsAtEvent = false;
            GetMutableDefault<URebuildProbeStyleA>()->SingleMaterialBrush.TintColor = FSlateColor(FLinearColor::White);
            NormalStyle = FButtonStyle();
            SelectedStyle = FButtonStyle();
            DisabledStyle = FButtonStyle();
            NativeInternalButton.Get()->SetStyle(FButtonStyle());
            MinWidth = 2551;
            MinHeight = 2601;
        }
        if (bSwapAtEvent)
        {
            bSwapAtEvent = false;
            Style = URebuildProbeStyleB::StaticClass();
            NativeInternalButton = ReplacementInternal;
            MinWidth = 3;
            MinHeight = 7;
            if (bReenterAtEvent)
            {
                bReenterAtEvent = false;
                NativeRebuildStyle();
            }
        }
    }
}

namespace
{
    struct FLegacyProbeSound : FSlateSound
    {
        static FSlateSound Named(UObject* Object)
        {
            FLegacyProbeSound Value;
            Value.LegacyResourceName_DEPRECATED = FName(*Object->GetPathName());
            Value.LegacyResourceObject_DEPRECATED.Reset();
            return Value;
        }
        static FSlateSound WithLegacyCache(UObject* Object, bool Cached)
        {
            FLegacyProbeSound Value;
            Value.ResourceObject = Object;
            Value.LegacyResourceName_DEPRECATED = FName(*Object->GetPathName());
            Value.LegacyResourceObject_DEPRECATED = Cached ? Object : nullptr;
            return Value;
        }
    };

    struct FReleaseState { bool Result = false; };
    struct FReleaseMutation
    {
        TWeakObjectPtr<UCommonButtonStyle> Style;
        TWeakObjectPtr<UCommonButton> Button;
        TSharedRef<FReleaseState> State;
        void operator()(FLinearColor* Color) const
        {
            State->Result = true;
            if (UCommonButtonStyle* Live = Style.Get()) Live->bSingleMaterial = true;
            if (UCommonButton* Live = Button.Get()) Live->PressedSlateSoundOverride = FSlateSound();
            delete Color;
        }
    };

    struct FReleaseObservation
    {
        TWeakObjectPtr<UCommonButtonInternal> Internal;
        TSharedRef<FReleaseState> State;
        void operator()(FLinearColor* Color) const
        {
            UCommonButtonInternal* Live = Internal.Get();
            State->Result = Live && Live->MinWidth == 2551 && Live->MinHeight == 2601;
            delete Color;
        }
    };

    // These are ordinary base-copy constructions of valid derived objects,
    // never downcasts of base objects. They expose protected semantic fields
    // without reading padding or triggering legacy-resource resolution.
    struct FSoundSnapshot : FSlateSound
    {
        explicit FSoundSnapshot(const FSlateSound& Value) : FSlateSound(Value) {}
        static bool Equal(const FSlateSound& A, const FSlateSound& B)
        {
            const FSoundSnapshot Left(A), Right(B);
            return Left.ResourceObject == Right.ResourceObject
                && Left.LegacyResourceName_DEPRECATED == Right.LegacyResourceName_DEPRECATED
                && Left.LegacyResourceObject_DEPRECATED.HasSameIndexAndSerialNumber(Right.LegacyResourceObject_DEPRECATED);
        }
        static bool IsResolvedLegacyCopy(const FSlateSound& Sound, UObject* Object)
        {
            const FSoundSnapshot Value(Sound);
            return Value.ResourceObject == nullptr
                && Value.LegacyResourceName_DEPRECATED == FName(*Object->GetPathName())
                && Value.LegacyResourceObject_DEPRECATED.HasSameIndexAndSerialNumber(TWeakObjectPtr<UObject>(Object));
        }
        static bool HasRawResource(const FSlateSound& Sound)
        {
            return FSoundSnapshot(Sound).ResourceObject != nullptr;
        }
        static UObject* RawResource(const FSlateSound& Sound)
        {
            return FSoundSnapshot(Sound).ResourceObject;
        }
    };

    struct FBrushSnapshot : FSlateBrush
    {
        explicit FBrushSnapshot(const FSlateBrush& Value) : FSlateBrush(Value) {}
        static FSlateBrush Marked(int32 Marker)
        {
            FBrushSnapshot Value{FSlateBrush()};
            Value.ImageType = Marker % 2 ? ESlateBrushImageType::Linear : ESlateBrushImageType::FullColor;
            Value.bHasUObject_DEPRECATED = !!(Marker & 1);
            Value.bIsDynamicallyLoaded = !!(Marker & 2);
            Value.UVRegion = FBox2D(FVector2D(0.01f * Marker, 0.02f), FVector2D(0.8f, 0.9f));
            return Value;
        }
        static bool Equal(const FSlateBrush& A, const FSlateBrush& B)
        {
            const FBrushSnapshot Left(A), Right(B);
            return A == B && Left.ImageType == Right.ImageType
                && Left.bHasUObject_DEPRECATED == Right.bHasUObject_DEPRECATED
                && Left.bIsDynamicallyLoaded == Right.bIsDynamicallyLoaded
                && Left.ResourceName == Right.ResourceName && Left.UVRegion == Right.UVRegion
                && Left.ResourceHandle.IsValid() == Right.ResourceHandle.IsValid()
                && Left.ResourceHandle.GetResourceProxy() == Right.ResourceHandle.GetResourceProxy();
        }
    };

    bool BrushEqual(const FSlateBrush& A, const FSlateBrush& B) { return FBrushSnapshot::Equal(A, B); }
    bool SoundEqual(const FSlateSound& A, const FSlateSound& B) { return FSoundSnapshot::Equal(A, B); }
    bool StyleEqual(const FButtonStyle& A, const FButtonStyle& B)
    {
        return BrushEqual(A.Normal, B.Normal) && BrushEqual(A.Hovered, B.Hovered)
            && BrushEqual(A.Pressed, B.Pressed) && BrushEqual(A.Disabled, B.Disabled)
            && A.NormalPadding == B.NormalPadding && A.PressedPadding == B.PressedPadding
            && SoundEqual(A.PressedSlateSound, B.PressedSlateSound)
            && SoundEqual(A.HoveredSlateSound, B.HoveredSlateSound);
    }
}

bool RunStyleRebuildProbe(TFunctionRef<void(const TCHAR*, bool)> Record)
{
    bool All = true;
    auto Check = [&All, &Record](const TCHAR* Name, bool Pass) { All &= Pass; Record(Name, Pass); };
    URebuildProbeStyleA* A = GetMutableDefault<URebuildProbeStyleA>();
    URebuildProbeStyleB* B = GetMutableDefault<URebuildProbeStyleB>();
    UCommonTextStyle* Text = GetMutableDefault<URebuildProbeText>();
    Text->Font.Size = 37;
    A->NormalTextStyle = URebuildProbeText::StaticClass();
    B->NormalTextStyle = URebuildProbeText::StaticClass();
    URebuildProbeButton* Button = NewObject<URebuildProbeButton>();
    UCommonButtonInternal* I = NewObject<UCommonButtonInternal>(Button);
    UCommonButtonInternal* J = NewObject<UCommonButtonInternal>(Button);
    TStrongObjectPtr<URebuildProbeButton> KeepButton(Button);
    TStrongObjectPtr<UCommonButtonInternal> KeepI(I), KeepJ(J);
    TSharedRef<SWidget> SlateI = I->TakeWidget();
    TSharedRef<SWidget> SlateJ = J->TakeWidget();
    FCommonButtonStyleRebuildProbeAccess::Bind(*Button, I);
    FCommonButtonStyleRebuildProbeAccess::State(*Button, false, true);
    Button->Style = A->GetClass();
    A->bSingleMaterial = false;
    A->ButtonPadding = FMargin(1, 2, 3, 4);
    A->MinWidth = 501; A->MinHeight = 601;
    B->MinWidth = 1501; B->MinHeight = 1601;
    Button->MinWidth = 551; Button->MinHeight = 501;
    FSlateBrush* Brushes[] = {&A->NormalBase, &A->NormalHovered, &A->NormalPressed,
        &A->SelectedBase, &A->SelectedHovered, &A->SelectedPressed, &A->Disabled};
    for (int32 N = 0; N < UE_ARRAY_COUNT(Brushes); ++N)
    {
        *Brushes[N] = FBrushSnapshot::Marked(N);
        Brushes[N]->ImageSize = FVector2D(10 + N, 20 + N);
        Brushes[N]->TintColor = FSlateColor(FLinearColor(0.1f * N, 0.2f, 0.3f, 1));
        Brushes[N]->Margin = FMargin(N);
    }
    FSlateSound Sounds[8];
    TStrongObjectPtr<USoundWave> KeepSounds[8];
    for (int32 Index = 0; Index < 8; ++Index) {
        KeepSounds[Index].Reset(NewObject<USoundWave>(Button));
        Sounds[Index].SetResourceObject(KeepSounds[Index].Get());
    }
    const FSlateSound UncachedLegacy = FLegacyProbeSound::WithLegacyCache(FSoundSnapshot::RawResource(Sounds[6]), false);
    const FSlateSound CachedLegacy = FLegacyProbeSound::WithLegacyCache(FSoundSnapshot::RawResource(Sounds[6]), true);
    Check(TEXT("sound_comparison_detects_legacy_name_and_cache"), !SoundEqual(Sounds[6], UncachedLegacy)
        && !SoundEqual(UncachedLegacy, CachedLegacy));
    FSlateBrush OtherImageType = A->NormalBase;
    OtherImageType.ImageType = ESlateBrushImageType::NoImage;
    Check(TEXT("brush_comparison_detects_image_type"), A->NormalBase == OtherImageType && !BrushEqual(A->NormalBase, OtherImageType));
    A->PressedSlateSound = Sounds[0]; A->HoveredSlateSound = Sounds[1];
    A->SelectedPressedSlateSound.Sound = Sounds[2]; A->SelectedHoveredSlateSound.Sound = Sounds[3];
    A->DisabledPressedSlateSound.Sound = Sounds[4]; A->DisabledHoveredSlateSound.Sound = Sounds[5];
    // Full independent presence/optional-flag matrix uses actual FSlateSound calls.
    for (int32 Mask = 0; Mask < 64; ++Mask)
    {
        Button->PressedSlateSoundOverride = Mask & 1 ? Sounds[6] : FSlateSound();
        Button->HoveredSlateSoundOverride = Mask & 2 ? Sounds[7] : FSlateSound();
        A->SelectedPressedSlateSound.bHasSound = !!(Mask & 4);
        A->SelectedHoveredSlateSound.bHasSound = !!(Mask & 8);
        A->DisabledPressedSlateSound.bHasSound = !!(Mask & 16);
        A->DisabledHoveredSlateSound.bHasSound = !!(Mask & 32);
        FCommonButtonStyleRebuildProbeAccess::Rebuild(*Button);
        const FSlateSound& P = Mask & 1 ? Sounds[6] : Sounds[0];
        const FSlateSound& H = Mask & 2 ? Sounds[7] : Sounds[1];
        const bool Pass = SoundEqual(Button->NormalStyle.PressedSlateSound, P)
            && SoundEqual(Button->NormalStyle.HoveredSlateSound, H)
            && SoundEqual(Button->SelectedStyle.PressedSlateSound, !(Mask & 1) && (Mask & 4) ? Sounds[2] : P)
            && SoundEqual(Button->SelectedStyle.HoveredSlateSound, !(Mask & 2) && (Mask & 8) ? Sounds[3] : H)
            && SoundEqual(Button->DisabledStyle.PressedSlateSound, !(Mask & 1) && (Mask & 16) ? Sounds[4] : P)
            && SoundEqual(Button->DisabledStyle.HoveredSlateSound, !(Mask & 2) && (Mask & 32) ? Sounds[5] : H);
        Check(*FString::Printf(TEXT("sound_matrix_%02d"), Mask), Pass);
    }
    Check(TEXT("normal_brush_mapping"), BrushEqual(Button->NormalStyle.Normal, A->NormalBase)
        && BrushEqual(Button->NormalStyle.Hovered, A->NormalHovered) && BrushEqual(Button->NormalStyle.Pressed, A->NormalPressed)
        && BrushEqual(Button->NormalStyle.Disabled, A->Disabled));
    Check(TEXT("selected_brush_mapping"), BrushEqual(Button->SelectedStyle.Normal, A->SelectedBase)
        && BrushEqual(Button->SelectedStyle.Hovered, A->SelectedHovered) && BrushEqual(Button->SelectedStyle.Pressed, A->SelectedPressed)
        && BrushEqual(Button->SelectedStyle.Disabled, A->Disabled));
    Check(TEXT("disabled_inherits_normal"), BrushEqual(Button->DisabledStyle.Normal, Button->NormalStyle.Normal)
        && BrushEqual(Button->DisabledStyle.Hovered, Button->NormalStyle.Hovered)
        && BrushEqual(Button->DisabledStyle.Pressed, Button->NormalStyle.Pressed)
        && BrushEqual(Button->DisabledStyle.Disabled, Button->NormalStyle.Disabled));
    Check(TEXT("padding"), Button->NormalStyle.NormalPadding == A->ButtonPadding && Button->NormalStyle.PressedPadding == A->ButtonPadding
        && Button->SelectedStyle.NormalPadding == A->ButtonPadding && Button->SelectedStyle.PressedPadding == A->ButtonPadding
        && Button->DisabledStyle.NormalPadding == A->ButtonPadding && Button->DisabledStyle.PressedPadding == A->ButtonPadding);
    SlateI->SlatePrepass(1);
    Check(TEXT("actual_sbox_dimensions"), I->MinWidth == 551 && I->MinHeight == 601
        && SlateI->GetDesiredSize().X >= 551 && SlateI->GetDesiredSize().Y >= 601);
    Check(TEXT("actual_reflected_event_font"), Button->NativeEvents == 64 && Button->ReflectedEvents == 64 && Button->ObservedFontSize == 37);
    for (int32 Mask = 0; Mask < 4; ++Mask)
    {
        FCommonButtonStyleRebuildProbeAccess::State(*Button, !!(Mask & 1), !!(Mask & 2));
        FCommonButtonStyleRebuildProbeAccess::Refresh(*Button);
        const FButtonStyle& Expected = Mask & 1 ? Button->SelectedStyle : Mask & 2 ? Button->NormalStyle : Button->DisabledStyle;
        Check(*FString::Printf(TEXT("live_style_%d"), Mask), StyleEqual(I->WidgetStyle, Expected));
    }
    FCommonButtonStyleRebuildProbeAccess::State(*Button, false, true);
    Button->PressedSlateSoundOverride = FLegacyProbeSound::Named(FSoundSnapshot::RawResource(Sounds[6]));
    Button->HoveredSlateSoundOverride = FLegacyProbeSound::Named(FSoundSnapshot::RawResource(Sounds[7]));
    FCommonButtonStyleRebuildProbeAccess::Rebuild(*Button);
    Check(TEXT("legacy_sound_lookup_and_copy_without_comparison_resolution"),
        FSoundSnapshot::IsResolvedLegacyCopy(Button->PressedSlateSoundOverride, FSoundSnapshot::RawResource(Sounds[6]))
        && FSoundSnapshot::IsResolvedLegacyCopy(Button->HoveredSlateSoundOverride, FSoundSnapshot::RawResource(Sounds[7]))
        && SoundEqual(Button->NormalStyle.PressedSlateSound, Button->PressedSlateSoundOverride)
        && SoundEqual(Button->NormalStyle.HoveredSlateSound, Button->HoveredSlateSoundOverride)
        && SoundEqual(Button->SelectedStyle.PressedSlateSound, Button->PressedSlateSoundOverride)
        && SoundEqual(Button->SelectedStyle.HoveredSlateSound, Button->HoveredSlateSoundOverride)
        && SoundEqual(Button->DisabledStyle.PressedSlateSound, Button->PressedSlateSoundOverride)
        && SoundEqual(Button->DisabledStyle.HoveredSlateSound, Button->HoveredSlateSoundOverride));
    A->bSingleMaterial = true;
    A->SingleMaterialBrush = A->NormalBase;
    A->SingleMaterialBrush.SetResourceObject(UMaterial::GetDefaultMaterial(MD_Surface));
    FCommonButtonStyleRebuildProbeAccess::Rebuild(*Button);
    UMaterialInstanceDynamic* MID = Button->SingleMaterialStyleMID;
    Check(TEXT("actual_mid_parent_outer"), MID && MID->GetOuter() == Button && MID->Parent == A->SingleMaterialBrush.GetResourceObject());
    FSlateBrush ExpectedSingle = A->SingleMaterialBrush;
    if (MID) ExpectedSingle.SetResourceObject(MID);
    Check(TEXT("single_material_all_brushes"), MID && BrushEqual(Button->NormalStyle.Normal, ExpectedSingle)
        && BrushEqual(Button->NormalStyle.Hovered, ExpectedSingle) && BrushEqual(Button->NormalStyle.Pressed, ExpectedSingle)
        && BrushEqual(Button->NormalStyle.Disabled, ExpectedSingle) && BrushEqual(Button->SelectedStyle.Normal, ExpectedSingle)
        && BrushEqual(Button->SelectedStyle.Hovered, ExpectedSingle) && BrushEqual(Button->SelectedStyle.Pressed, ExpectedSingle)
        && BrushEqual(Button->SelectedStyle.Disabled, ExpectedSingle) && BrushEqual(Button->DisabledStyle.Normal, ExpectedSingle)
        && BrushEqual(Button->DisabledStyle.Hovered, ExpectedSingle) && BrushEqual(Button->DisabledStyle.Pressed, ExpectedSingle)
        && BrushEqual(Button->DisabledStyle.Disabled, ExpectedSingle));
    A->SingleMaterialBrush.SetResourceObject(NewObject<UTexture2D>(Button));
    FCommonButtonStyleRebuildProbeAccess::Rebuild(*Button);
    Check(TEXT("single_nonmaterial_no_mid"), !Button->SingleMaterialStyleMID && BrushEqual(Button->NormalStyle.Normal, A->SingleMaterialBrush));
    TSharedRef<FReleaseState> ReleasedAfterDimensions = MakeShared<FReleaseState>();
    {
        TSharedRef<FLinearColor> Linked = MakeShareable(new FLinearColor(FLinearColor::White), FReleaseObservation{TWeakObjectPtr<UCommonButtonInternal>(I), ReleasedAfterDimensions});
        A->SingleMaterialBrush.TintColor = FSlateColor(Linked);
    }
    Button->bClearBrushColorsAtEvent = true;
    FCommonButtonStyleRebuildProbeAccess::Rebuild(*Button);
    Check(TEXT("temporary_brush_destruction_after_event_and_dimensions"), ReleasedAfterDimensions->Result);
    Button->MinWidth = 551;
    Button->MinHeight = 501;
    A->bSingleMaterial = false;
    TSharedRef<FReleaseState> Released = MakeShared<FReleaseState>();
    {
        TSharedRef<FLinearColor> Linked = MakeShareable(new FLinearColor(FLinearColor::White), FReleaseMutation{TWeakObjectPtr<UCommonButtonStyle>(A), TWeakObjectPtr<UCommonButton>(Button), Released});
        Button->NormalStyle.Normal.TintColor = FSlateColor(Linked);
    }
    Button->PressedSlateSoundOverride = Sounds[6];
    Button->HoveredSlateSoundOverride = FSlateSound();
    FCommonButtonStyleRebuildProbeAccess::Rebuild(*Button);
    Check(TEXT("real_shared_release_rereads_single"), Released->Result && BrushEqual(Button->NormalStyle.Normal, A->NormalBase)
        && BrushEqual(Button->NormalStyle.Hovered, FSlateBrush()) && BrushEqual(Button->SelectedStyle.Normal, FSlateBrush()));
    Check(TEXT("override_presence_cached_before_brush_release"), !FSoundSnapshot::HasRawResource(Button->NormalStyle.PressedSlateSound)
        && SoundEqual(Button->NormalStyle.PressedSlateSound, Button->PressedSlateSoundOverride)
        && SoundEqual(Button->SelectedStyle.PressedSlateSound, Button->PressedSlateSoundOverride)
        && SoundEqual(Button->DisabledStyle.PressedSlateSound, Button->PressedSlateSoundOverride));
    A->bSingleMaterial = false;
    Button->ReplacementInternal = J;
    Button->bSwapAtEvent = true;
    FCommonButtonStyleRebuildProbeAccess::Rebuild(*Button);
    SlateJ->SlatePrepass(1);
    Check(TEXT("event_re_resolves_cdo_and_weak"), Button->Style == B->GetClass() && J->MinWidth == 1501 && J->MinHeight == 1601
        && I->MinWidth == 551 && I->MinHeight == 601 && SlateJ->GetDesiredSize().X >= 1501 && SlateJ->GetDesiredSize().Y >= 1601);
    Button->Style = A->GetClass();
    FCommonButtonStyleRebuildProbeAccess::Bind(*Button, I);
    const int32 Before = Button->ReflectedEvents;
    Button->bSwapAtEvent = true;
    Button->bReenterAtEvent = true;
    FCommonButtonStyleRebuildProbeAccess::Rebuild(*Button);
    Check(TEXT("actual_event_synchronous_reentry"), Button->ReflectedEvents == Before + 2 && J->MinWidth == 1501 && J->MinHeight == 1601);

    // Same dynamic-class construction sequence as UE4.25
    // ConfigEditorPropertyDetails.cpp:37-51, without its later explicit CDO creation.
    TStrongObjectPtr<UClass> MissingCDOClass(NewObject<UClass>(GetTransientPackage()));
    MissingCDOClass->SetSuperStruct(UCommonButtonStyle::StaticClass());
    MissingCDOClass->Bind();
    MissingCDOClass->StaticLink(true);
    Button->Style = MissingCDOClass.Get();
    const int32 MissingBeforeNative = Button->NativeEvents;
    const int32 MissingBeforeReflected = Button->ReflectedEvents;
    const FButtonStyle MissingBeforeNormal = Button->NormalStyle;
    const FButtonStyle MissingBeforeSelected = Button->SelectedStyle;
    const FButtonStyle MissingBeforeDisabled = Button->DisabledStyle;
    UMaterialInstanceDynamic* MissingBeforeMID = Button->SingleMaterialStyleMID;
    const int32 MissingBeforeWidth = J->MinWidth;
    const int32 MissingBeforeHeight = J->MinHeight;
    Check(TEXT("valid_style_class_cdo_absent_before"), MissingCDOClass->IsChildOf(UCommonButtonStyle::StaticClass())
        && Button->Style.Get() == MissingCDOClass.Get() && !MissingCDOClass->GetDefaultObject(false));
    Check(TEXT("style_cdo_resolver_does_not_create"), !FCommonButtonStyleRebuildProbeAccess::CDO(*Button)
        && !MissingCDOClass->GetDefaultObject(false));
    FCommonButtonStyleRebuildProbeAccess::Rebuild(*Button);
    Check(TEXT("missing_cdo_rebuild_no_creation_or_effects"), !MissingCDOClass->GetDefaultObject(false)
        && Button->NativeEvents == MissingBeforeNative && Button->ReflectedEvents == MissingBeforeReflected
        && StyleEqual(Button->NormalStyle, MissingBeforeNormal) && StyleEqual(Button->SelectedStyle, MissingBeforeSelected)
        && StyleEqual(Button->DisabledStyle, MissingBeforeDisabled) && Button->SingleMaterialStyleMID == MissingBeforeMID
        && J->MinWidth == MissingBeforeWidth && J->MinHeight == MissingBeforeHeight);
    Button->Style = nullptr;
    const int32 NoStyleBefore = Button->ReflectedEvents;
    const FSlateBrush Saved = Button->NormalStyle.Normal;
    FCommonButtonStyleRebuildProbeAccess::Rebuild(*Button);
    Check(TEXT("null_style_no_refresh"), Button->ReflectedEvents == NoStyleBefore && BrushEqual(Button->NormalStyle.Normal, Saved));
    FCommonButtonStyleRebuildProbeAccess::Bind(*Button, nullptr);
    FCommonButtonStyleRebuildProbeAccess::Refresh(*Button);
    Check(TEXT("weak_missing_refresh_has_no_event"), Button->ReflectedEvents == NoStyleBefore);
    // Do not call full rebuild with valid Style and an unbound internal: original
    // dimension refresh has an unguarded dereference, not a supported no-op.
    return All;
}
