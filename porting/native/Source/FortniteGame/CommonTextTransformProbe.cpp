#include "CommonTextTransformProbe.h"
#include "UCommonTextStyle.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Framework/Text/PlainTextLayoutMarshaller.h"
#include "Framework/Text/SlateTextLayout.h"
#include "Internationalization/TextTransformer.h"
#include "Widgets/Text/SlateTextBlockLayout.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

namespace TextTransformDiagnostic
{
int32 Sequence = 0; bool Failed = false, Registered = false; double Deadline = 0; FDelegateHandle FrameHandle;
void Record(const TCHAR* Event, bool OK) { Failed |= !OK; UE_LOG(LogTemp, Display, TEXT("TEXTTRANSFORM v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK)); }
bool Metadata(UFunction* F)
{
    if (!F || F->GetOuterUClass() != UCommonTextBlock::StaticClass() || uint32(F->FunctionFlags) != 0x04020401 || !F->GetNativeFunc() || F->NumParms != 1 || F->ParmsSize != sizeof(bool) || F->GetReturnProperty()) return false;
    FBoolProperty* P = CastField<FBoolProperty>(F->PropertyLink);
    return P && P->GetFName() == FName(TEXT("bUseAllCaps")) && P->GetOffset_ForInternal() == 0 && P->HasAllPropertyFlags(CPF_Parm) && !P->HasAnyPropertyFlags(CPF_OutParm | CPF_ReturnParm) && P->IsNativeBool();
}
bool Lines(const TSharedPtr<FSlateTextLayout>& L, const TCHAR* A, const TCHAR* B = nullptr)
{
    if (!L.IsValid()) return false; const auto& M = L->GetLineModels();
    return M.Num() == (B ? 2 : 1) && *M[0].Text == A && (!B || *M[1].Text == B);
}
void Run(APlayerController* PC)
{
    Record(TEXT("begin"), true);
    TStrongObjectPtr<UTextTransformProbeBlock> W(NewObject<UTextTransformProbeBlock>(PC));
    UCommonTextBlock* CDO = GetMutableDefault<UCommonTextBlock>();
    UEnum* E = FindObject<UEnum>(ANY_PACKAGE, TEXT("ETextTransformPolicy"));
    FProperty* Prop = FindFProperty<FProperty>(UCommonTextBlock::StaticClass(), TEXT("TextTransformPolicy"));
    Record(TEXT("reflection_enum_property"), E && E->GetOutermost()->GetName() == TEXT("/Script/Slate") && E->GetValueByNameString(TEXT("None")) == 0 && E->GetValueByNameString(TEXT("ToLower")) == 1 && E->GetValueByNameString(TEXT("ToUpper")) == 2 && Prop && Prop->GetOwner<UClass>() == UTextBlock::StaticClass());
    FEnumProperty* EnumProp = CastField<FEnumProperty>(Prop);
    Record(TEXT("enum_storage"), EnumProp && EnumProp->GetEnum() == E && EnumProp->GetUnderlyingProperty()->GetSize() == 1);
    UFunction* Fn = W->FindFunction(TEXT("SetTextCase")); const bool Meta = Metadata(Fn);
    Record(TEXT("metadata"), Meta);
    Record(TEXT("cdo_default"), CDO && CDO->TextTransformPolicy == ETextTransformPolicy::None && W->TextTransformPolicy == ETextTransformPolicy::None);
    const FText Raw = NSLOCTEXT("TextTransformProbe", "Raw", "mmmm"); W->Text = Raw; W->bDisplayAllCaps = true;
    int32 N = W->SyncCount; W->SetTextCase(true);
    Record(TEXT("true_no_slate"), !W->HasSlate() && W->SyncCount == N+1 && W->TextTransformPolicy == ETextTransformPolicy::ToUpper && W->bDisplayAllCaps && W->Text.IdenticalTo(Raw));
    N = W->SyncCount; W->SetTextCase(true);
    Record(TEXT("repeat_true"), W->SyncCount == N+1 && W->TextTransformPolicy == ETextTransformPolicy::ToUpper);
    W->TextTransformPolicy = ETextTransformPolicy::ToLower; N = W->SyncCount;
    struct { bool bUseAllCaps; } Params = {false}; if (Meta) W->ProcessEvent(Fn, &Params);
    Record(TEXT("processevent_false_after_lower"), Meta && W->SyncCount == N+1 && W->TextTransformPolicy == ETextTransformPolicy::None && W->bDisplayAllCaps && W->Text.IdenticalTo(Raw));
    N = W->SyncCount; W->SetTextCase(false);
    Record(TEXT("repeat_false"), W->SyncCount == N+1 && W->TextTransformPolicy == ETextTransformPolicy::None);
    W->SetTextCase(true); W->TakeWidget();
    const FVector2D USize = W->SlateSize(); N = W->SyncCount; W->SetTextCase(false); const FVector2D USize2 = W->SlateSize();
    Record(TEXT("live_widget"), W->HasSlate() && W->SyncCount == N+1 && W->Text.IdenticalTo(Raw) && W->SlateText().IdenticalTo(Raw) && W->bDisplayAllCaps && USize.X > 0 && USize.Y > 0 && USize2.X > 0 && USize2.Y > 0 && !FMath::IsNearlyEqual(USize.X, USize2.X, .01f));

    // The widget's cache is private. This second real cache uses the same widget args and
    // marshaller, with a factory that only retains the actual FSlateTextLayout it creates.
    FText Source = NSLOCTEXT("TextTransformProbe", "Bound", "MiXeD iI\nSeCoNd");
    const FText Initial = Source; ETextTransformPolicy Policy = ETextTransformPolicy::None;
    auto TextAttr = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateLambda([&Source]() { return Source; }));
    auto PolicyAttr = TAttribute<ETextTransformPolicy>::Create(TAttribute<ETextTransformPolicy>::FGetter::CreateLambda([&Policy]() { return Policy; }));
    TSharedRef<STextBlock> S = SNew(STextBlock).Text(TextAttr).TransformPolicy(PolicyAttr).SimpleTextMode(false);
    TSharedRef<STextBlock> PolicyOnly = SNew(STextBlock).Text(FText::FromString(TEXT("constant"))).TransformPolicy(PolicyAttr);
    PolicyOnly->CacheVolatility();
    Record(TEXT("bound_policy_volatile"), PolicyOnly->IsVolatile());
    TSharedPtr<FSlateTextLayout> Captured;
    auto Factory = FCreateSlateTextLayout::CreateLambda([&Captured](SWidget* Owner, const FTextBlockStyle& Style) { auto L = FSlateTextLayout::Create(Owner, Style); Captured = L; return L; });
    UClass* Asset = LoadClass<UCommonTextStyle>(nullptr, TEXT("/Game/UI/Foundation/Buttons/TextStyles/TextStyle-Button-Primary-M.TextStyle-Button-Primary-M_C"));
    UCommonTextStyle* AssetCDO = Asset ? Cast<UCommonTextStyle>(Asset->GetDefaultObject()) : nullptr;
    FTextBlockStyle Style = FTextBlockStyle::GetDefault(); if (AssetCDO) Style.SetFont(AssetCDO->Font);
    if (AssetCDO) UE_LOG(LogTemp, Display, TEXT("TEXT_TRANSFORM_ASSET style=%s"), *Asset->GetPathName());
    Record(TEXT("original_font"), Asset && Asset->HasAnyFlags(RF_WasLoaded) && AssetCDO && Style.Font == AssetCDO->Font);
    FSlateTextBlockLayout Cache(&S.Get(), Style, TOptional<ETextShapingMethod>(), TOptional<ETextFlowDirection>(), Factory, FPlainTextLayoutMarshaller::Create(), nullptr);
    TAttribute<FText> Highlight(FText::GetEmpty()); TAttribute<float> Wrap(0.0f), Height(1.0f); TAttribute<bool> Auto(false);
    TAttribute<ETextWrappingPolicy> Wrapping(ETextWrappingPolicy::DefaultWrapping); TAttribute<FMargin> Margin(FMargin(0)); TAttribute<ETextJustify::Type> Justify(ETextJustify::Left);
    auto Measure = [&]() { S->SlatePrepass(1); return Cache.ComputeDesiredSize(FSlateTextBlockLayout::FWidgetArgs(TextAttr, Highlight, Wrap, Auto, Wrapping, PolicyAttr, Margin, Height, Justify), 1, Style); };
    FVector2D SizeNone = Measure();
    Record(TEXT("layout_none_multiline"), Lines(Captured, TEXT("MiXeD iI"), TEXT("SeCoNd")) && S->GetText().IdenticalTo(Initial) && SizeNone.X > 0 && SizeNone.Y > 0);
    Policy = ETextTransformPolicy::ToLower; FVector2D SizeLower = Measure();
    Record(TEXT("layout_lower"), Lines(Captured, TEXT("mixed ii"), TEXT("second")) && S->GetText().IdenticalTo(Initial) && SizeLower.X > 0);
    Policy = ETextTransformPolicy::ToUpper; FVector2D SizeUpper = Measure();
    Record(TEXT("layout_upper"), Lines(Captured, TEXT("MIXED II"), TEXT("SECOND")) && S->GetText().IdenticalTo(Initial) && SizeUpper.X > 0);
    Policy = ETextTransformPolicy::None; FVector2D SizeRestored = Measure();
    Record(TEXT("layout_restore_cache"), Lines(Captured, TEXT("MiXeD iI"), TEXT("SeCoNd")) && S->GetText().IdenticalTo(Initial) && SizeRestored.Equals(SizeNone, .01f));
    TSharedRef<STextBlock> Direct = SNew(STextBlock).Text(FText::FromString(TEXT("mmmm"))).TextStyle(&Style).SimpleTextMode(false);
    Direct->SlatePrepass(1); const FVector2D DirectNone = Direct->GetDesiredSize();
    Direct->SetTransformPolicy(ETextTransformPolicy::ToUpper); Direct->SlatePrepass(1); const FVector2D DirectUpper = Direct->GetDesiredSize();
    Direct->SetTransformPolicy(ETextTransformPolicy::None); Direct->SlatePrepass(1);
    Record(TEXT("widget_geometry_transition"), DirectNone.X > 0 && DirectUpper.X > 0 && !FMath::IsNearlyEqual(DirectNone.X, DirectUpper.X, .01f) && Direct->GetDesiredSize().Equals(DirectNone, .01f) && Direct->GetText().ToString() == TEXT("mmmm"));
    Source = NSLOCTEXT("TextTransformProbe", "Changed", "NeW text"); Policy = ETextTransformPolicy::ToUpper; Measure();
    Record(TEXT("bound_source_change"), Lines(Captured, TEXT("NEW TEXT")) && S->GetText().IdenticalTo(Source) && FTextInspector::GetNamespace(Source) == FTextInspector::GetNamespace(S->GetText()) && FTextInspector::GetKey(Source) == FTextInspector::GetKey(S->GetText()));
    Source = FText::FromString(TEXT("\u00e9cole")); Measure();
    Record(TEXT("unicode_same_length"), Lines(Captured, TEXT("\u00c9COLE")) && S->GetText().IdenticalTo(Source));
    // Only assert refusal when this runtime converter really expands this input.
    const FString Expansion(TEXT("\u00df")); const FString Converted = FTextTransformer::ToUpper(Expansion);
    if (Converted.Len() != Expansion.Len()) { Source = FText::FromString(Expansion); Measure(); Record(TEXT("unicode_length_refusal"), Lines(Captured, *Expansion) && S->GetText().IdenticalTo(Source)); }
    else { UE_LOG(LogTemp, Display, TEXT("TEXTTRANSFORM v=1 event=unicode_length_refusal_unavailable source_len=%d transformed_len=%d"), Expansion.Len(), Converted.Len()); }
    TSharedRef<STextBlock> Simple = SNew(STextBlock).Text(FText::FromString(TEXT("MiXeD"))).SimpleTextMode(true);
    Simple->SlatePrepass(1); FVector2D SimpleNone = Simple->GetDesiredSize(); Simple->SetTransformPolicy(ETextTransformPolicy::ToUpper); Simple->SlatePrepass(1);
    Record(TEXT("simple_reference_measure_bypass"), Simple->GetText().ToString() == TEXT("MiXeD") && Simple->GetDesiredSize().Equals(SimpleNone, .01f));
}
void Finish() { FCoreDelegates::OnEndFrame.Remove(FrameHandle); Record(TEXT("complete"), !Failed); FPlatformMisc::RequestExitWithStatus(false, Failed ? 1 : 0); }
void Tick()
{
    if (FPlatformTime::Seconds() > Deadline) { Record(TEXT("timeout"), false); Finish(); return; }
    if (!GEngine) return;
    for (const FWorldContext& Context : GEngine->GetWorldContexts()) {
        UWorld* World = Context.World(); UGameInstance* GI = World ? World->GetGameInstance() : nullptr; ULocalPlayer* Player = GI ? GI->GetFirstGamePlayer() : nullptr;
        APlayerController* PC = Player ? Player->GetPlayerController(World) : nullptr;
        if (!World || !World->IsGameWorld() || !GI || !Player || !PC || PC->Player != Player || PC->GetWorld() != World || Player->GetGameInstance() != GI || GI->GetWorld() != World) continue;
        Run(PC); Finish(); return;
    }
}
}
void RegisterCommonTextTransformProbeDriver()
{
    using namespace TextTransformDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("CommonTextTransformProbe"))) return;
    Registered = true; Deadline = FPlatformTime::Seconds() + 180.; FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
