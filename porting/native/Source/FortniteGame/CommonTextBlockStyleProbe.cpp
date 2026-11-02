#include "CommonTextBlockStyleProbe.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

namespace TextBlockStyleDiagnostic
{
int32 Sequence = 0;
bool Failed = false, Registered = false;
double Deadline = 0;
FDelegateHandle FrameHandle;
void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("TEXTSTYLE v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
}
bool Metadata(UFunction* F)
{
    if (!F || F->GetOuterUClass() != UCommonTextBlock::StaticClass() || uint32(F->FunctionFlags) != 0x04020401
        || !F->GetNativeFunc() || F->NumParms != 1 || F->ParmsSize != 8 || F->GetReturnProperty()) return false;
    FClassProperty* P = CastField<FClassProperty>(F->PropertyLink);
    return P && P->GetFName() == FName(TEXT("InStyle")) && !P->HasAnyPropertyFlags(CPF_OutParm | CPF_ReturnParm) && P->GetOffset_ForInternal() == 0 && P->HasAllPropertyFlags(CPF_Parm)
        && P->PropertyClass == UClass::StaticClass() && P->MetaClass == UCommonTextStyle::StaticClass();
}
bool Applied(const UTextStyleProbeBlock* W, const UCommonTextStyle* S)
{
    return W->Font == S->Font && W->StrikeBrush == S->StrikeBrush && W->ProbeMargin() == S->Margin
        && W->ProbeLineHeight() == S->LineHeightPercentage && W->ColorAndOpacity == FSlateColor(S->Color)
        && W->ShadowOffset == (S->bUsesDropShadow ? S->ShadowOffset : FVector2D::ZeroVector)
        && W->ShadowColorAndOpacity == (S->bUsesDropShadow ? S->ShadowColor : FLinearColor::Transparent);
}
struct FState
{
    FSlateFontInfo Font; FSlateBrush Strike; FMargin Margin; float LineHeight; FSlateColor Color;
    FVector2D Shadow; FLinearColor ShadowColor;
    explicit FState(const UTextStyleProbeBlock* W) : Font(W->Font), Strike(W->StrikeBrush), Margin(W->ProbeMargin()),
        LineHeight(W->ProbeLineHeight()), Color(W->ColorAndOpacity), Shadow(W->ShadowOffset), ShadowColor(W->ShadowColorAndOpacity) {}
    bool Equals(const UTextStyleProbeBlock* W) const {
        return Font == W->Font && Strike == W->StrikeBrush && Margin == W->ProbeMargin() && LineHeight == W->ProbeLineHeight()
            && Color == W->ColorAndOpacity && Shadow == W->ShadowOffset && ShadowColor == W->ShadowColorAndOpacity;
    }
};
void Run(APlayerController* PC, UWorld* World)
{
    Record(TEXT("begin"), true);
    TStrongObjectPtr<UTextStyleProbeBlock> A(NewObject<UTextStyleProbeBlock>(PC));
    TStrongObjectPtr<UTextStyleProbeBlock> B(NewObject<UTextStyleProbeBlock>(PC));
    UCommonTextBlock* BaseCDO = GetMutableDefault<UCommonTextBlock>();
    UTextStyleProbeBlock* CDO = GetMutableDefault<UTextStyleProbeBlock>();
    UTextStyleProbeStyle* S = GetMutableDefault<UTextStyleProbeStyle>();
    Record(TEXT("context"), A.IsValid() && B.IsValid() && BaseCDO && CDO && S && PC->GetWorld() == World);
    if (!A.IsValid() || !B.IsValid() || !BaseCDO || !CDO || !S) return;
    const auto Defaults = [](const UCommonTextBlock* W) { return !W->Style && !W->ScrollStyle && !W->bDisplayAllCaps
        && !W->bAutoCollapseWithEmptyText && W->MobileFontSizeMultiplier == 1.0f; };
    Record(TEXT("defaults"), Defaults(BaseCDO) && Defaults(CDO) && Defaults(A.Get()) && Defaults(B.Get()));
    UFunction* Fn = A->FindFunction(TEXT("SetStyle"));
    const bool Meta = Metadata(Fn);
    Record(TEXT("metadata"), Meta);
    const FSlateFontInfo OldFont = S->Font; const FSlateBrush OldStrike = S->StrikeBrush;
    const FMargin OldMargin = S->Margin; const float OldLine = S->LineHeightPercentage;
    const FLinearColor OldColor = S->Color, OldShadowColor = S->ShadowColor;
    const FVector2D OldOffset = S->ShadowOffset; const bool OldDrop = S->bUsesDropShadow;
    S->Font = A->Font; S->Font.Size = 29; S->StrikeBrush.ImageSize = FVector2D(17, 23); S->Margin = FMargin(1, 2, 3, 4);
    S->LineHeightPercentage = 1.37f; S->Color = FLinearColor(.2f, .3f, .4f, .5f);
    S->ShadowOffset = FVector2D(6, 8); S->ShadowColor = FLinearColor(.7f, .6f, .5f, .4f); S->bUsesDropShadow = true;
    A->Text = FText::FromString(TEXT("raw one")); A->SetVisibility(ESlateVisibility::Hidden);
    int32 Count = A->SyncCount;
    A->SetStyle(UTextStyleProbeStyle::StaticClass());
    Record(TEXT("direct_shadow_on_no_slate"), !A->HasSlate() && A->SyncCount == Count + 1 && Applied(A.Get(), S)
        && A->GetVisibility() == ESlateVisibility::Hidden && A->Style.Get() == UTextStyleProbeStyle::StaticClass());
    struct { UClass* InStyle; } Params = {UTextStyleProbeStyle::StaticClass()};
    Count = B->SyncCount;
    if (Meta) B->ProcessEvent(Fn, &Params);
    Record(TEXT("processevent_shadow_on"), Meta && B->SyncCount == Count + 1 && Applied(B.Get(), S) && B->Style.Get() == Params.InStyle);
    S->bUsesDropShadow = false;
    Count = A->SyncCount; A->SetStyle(UTextStyleProbeStyle::StaticClass());
    Record(TEXT("repeated_shadow_off"), A->SyncCount == Count + 1 && Applied(A.Get(), S));
    FState Before(A.Get()); Count = A->SyncCount; A->SetStyle(nullptr);
    Record(TEXT("null_style"), !A->Style && A->SyncCount == Count + 1 && Before.Equals(A.Get()));
    TStrongObjectPtr<UClass> MissingStyle(NewObject<UClass>(GetTransientPackage()));
    MissingStyle->SetSuperStruct(UCommonTextStyle::StaticClass()); MissingStyle->Bind(); MissingStyle->StaticLink(true);
    Count = A->SyncCount; A->SetStyle(MissingStyle.Get());
    Record(TEXT("missing_style_cdo"), A->Style.Get() == MissingStyle.Get() && !MissingStyle->GetDefaultObject(false)
        && A->SyncCount == Count + 1 && Before.Equals(A.Get()));
    FClassProperty* StyleProperty = FindFProperty<FClassProperty>(UCommonTextBlock::StaticClass(), TEXT("Style"));
    if (StyleProperty) StyleProperty->SetObjectPropertyValue_InContainer(A.Get(), UObject::StaticClass());
    A->bAutoCollapseWithEmptyText = true; A->Text = FText::GetEmpty(); Count = A->SyncCount;
    A->SynchronizeProperties();
    Record(TEXT("wrong_class_storage"), StyleProperty && StyleProperty->GetObjectPropertyValue_InContainer(A.Get()) == UObject::StaticClass()
        && Before.Equals(A.Get()) && A->SyncCount == Count + 1 && A->GetVisibility() == ESlateVisibility::Collapsed);
    Params.InStyle = UObject::StaticClass(); Count = A->SyncCount;
    if (Meta) A->ProcessEvent(Fn, &Params);
    Record(TEXT("wrong_class_setter"), Meta && StyleProperty && StyleProperty->GetObjectPropertyValue_InContainer(A.Get()) == UObject::StaticClass()
        && A->SyncCount == Count + 1 && Before.Equals(A.Get()) && A->GetVisibility() == ESlateVisibility::Collapsed);
    // The original implementation examines raw Text, not the bound display text.
    A->TextDelegate.BindDynamic(A.Get(), &UTextStyleProbeBlock::BoundProbeText);
    A->Text = FText::GetEmpty(); A->SynchronizeProperties();
    Record(TEXT("raw_empty_with_delegate"), A->GetVisibility() == ESlateVisibility::Collapsed);
    A->Text = FText::FromString(TEXT("raw nonempty")); A->SynchronizeProperties();
    Record(TEXT("raw_nonempty"), A->GetVisibility() == ESlateVisibility::SelfHitTestInvisible);
    A->Text = FText::FromString(TEXT(" \t ")); A->SynchronizeProperties();
    Record(TEXT("whitespace_nonempty"), A->GetVisibility() == ESlateVisibility::SelfHitTestInvisible);
    A->TextDelegate.Unbind(); A->bAutoCollapseWithEmptyText = false; A->SetVisibility(ESlateVisibility::Visible);
    A->Text = FText::GetEmpty(); A->SynchronizeProperties();
    Record(TEXT("collapse_disabled"), A->GetVisibility() == ESlateVisibility::Visible);
    // TakeWidget constructs the actual STextBlock; changing raw Text avoids UTextBlock::SetText's immediate Slate write.
    A->TakeWidget();
    A->Text = FText::FromString(TEXT("parent sync text")); A->bAutoCollapseWithEmptyText = true;
    A->bCaptureVisibility = true; A->VisibilityCount = 0; S->bUsesDropShadow = true;
    Count = A->SyncCount; A->SetStyle(UTextStyleProbeStyle::StaticClass());
    Record(TEXT("rebuilt_parent_and_order"), A->HasSlate() && A->SyncCount == Count + 1 && Applied(A.Get(), S)
        && A->VisibilityCount == 1 && A->FontAtVisibility == S->Font && A->MarginAtVisibility == S->Margin
        && A->SlateTextAtVisibility.IsEmpty() && A->SlateText().EqualTo(A->Text) && A->GetVisibility() == ESlateVisibility::SelfHitTestInvisible);
    FState BeforeInvalidSlate(A.Get());
    if (StyleProperty) StyleProperty->SetObjectPropertyValue_InContainer(A.Get(), UObject::StaticClass());
    A->Text = FText::FromString(TEXT("invalid style still syncs"));
    Count = A->SyncCount; A->SynchronizeProperties();
    Record(TEXT("wrong_class_parent_slate"), StyleProperty && StyleProperty->GetObjectPropertyValue_InContainer(A.Get()) == UObject::StaticClass()
        && A->SyncCount == Count + 1 && BeforeInvalidSlate.Equals(A.Get()) && A->SlateText().EqualTo(A->Text)
        && A->GetVisibility() == ESlateVisibility::SelfHitTestInvisible);
    FState BeforeNullSlate(A.Get()); A->Text = FText::FromString(TEXT("null still syncs"));
    Count = A->SyncCount; A->SetStyle(nullptr);
    Record(TEXT("null_style_parent_slate"), !A->Style && A->SyncCount == Count + 1 && BeforeNullSlate.Equals(A.Get())
        && A->SlateText().EqualTo(A->Text) && A->GetVisibility() == ESlateVisibility::SelfHitTestInvisible);
    A->TextDelegate.BindDynamic(A.Get(), &UTextStyleProbeBlock::BoundProbeText);
    A->Text = FText::GetEmpty(); A->SetStyle(UTextStyleProbeStyle::StaticClass());
    Record(TEXT("bound_slate_raw_collapse"), A->SlateText().EqualTo(FText::FromString(TEXT("delegate text")))
        && A->GetVisibility() == ESlateVisibility::Collapsed);
    A->TextDelegate.Unbind(); A->Text = FText::FromString(TEXT("MMMM"));
    S->Font.Size = 14; A->SetStyle(UTextStyleProbeStyle::StaticClass()); const FVector2D Small = A->SlateDesired();
    S->Font.Size = 42; A->SetStyle(UTextStyleProbeStyle::StaticClass()); const FVector2D Large = A->SlateDesired();
    Record(TEXT("font_layout_updates"), Small.X > 0 && Small.Y > 0 && Large.X > Small.X && Large.Y > Small.Y && A->Font.Size == 42);
    UClass* Asset = LoadClass<UCommonTextStyle>(nullptr, TEXT("/Game/UI/Foundation/Buttons/TextStyles/TextStyle-Button-Primary-M.TextStyle-Button-Primary-M_C"));
    UCommonTextStyle* AssetCDO = Asset ? Cast<UCommonTextStyle>(Asset->GetDefaultObject()) : nullptr;
    if (AssetCDO) { A->SetStyle(Asset); UE_LOG(LogTemp, Display, TEXT("TEXT_BLOCK_STYLE_ASSET style=%s"), *Asset->GetPathName()); }
    Record(TEXT("original_asset"), Asset && Asset->HasAnyFlags(RF_WasLoaded) && AssetCDO && Applied(A.Get(), AssetCDO));
    S->Font = OldFont; S->StrikeBrush = OldStrike; S->Margin = OldMargin; S->LineHeightPercentage = OldLine;
    S->Color = OldColor; S->ShadowOffset = OldOffset; S->ShadowColor = OldShadowColor; S->bUsesDropShadow = OldDrop;
    Record(TEXT("fixture_restored"), S->Font == OldFont && S->StrikeBrush == OldStrike && S->Margin == OldMargin
        && S->LineHeightPercentage == OldLine && S->Color == OldColor && S->ShadowOffset == OldOffset
        && S->ShadowColor == OldShadowColor && S->bUsesDropShadow == OldDrop);
}
void Finish() { FCoreDelegates::OnEndFrame.Remove(FrameHandle); Record(TEXT("complete"), !Failed); FPlatformMisc::RequestExitWithStatus(false, Failed ? 1 : 0); }
void Tick()
{
    if (FPlatformTime::Seconds() > Deadline) { Record(TEXT("timeout"), false); Finish(); return; }
    if (!GEngine) return;
    for (const FWorldContext& Context : GEngine->GetWorldContexts()) {
        UWorld* World = Context.World(); UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
        ULocalPlayer* Player = GI ? GI->GetFirstGamePlayer() : nullptr;
        APlayerController* PC = Player ? Player->GetPlayerController(World) : nullptr;
        if (!World || !World->IsGameWorld() || !GI || !Player || !PC || PC->Player != Player
            || PC->GetWorld() != World || Player->GetGameInstance() != GI || GI->GetWorld() != World) continue;
        Run(PC, World); Finish(); return;
    }
}
}
void RegisterCommonTextBlockStyleProbeDriver()
{
    using namespace TextBlockStyleDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("CommonTextBlockStyleProbe"))) return;
    Registered = true; Deadline = FPlatformTime::Seconds() + 180.0;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
