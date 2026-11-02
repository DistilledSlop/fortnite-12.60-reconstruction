#include "CommonButtonTextStyleProbe.h"
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
#include "UObject/GarbageCollection.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"

// The production accessor reads this exact non-reflected slot. Only this opt-in
// diagnostic mutates it; no synthetic production selection API is introduced.
struct FCommonButtonTextStyleProbeAccess
{
    static void Set(UCommonButton* Button, bool Value) { Button->bNativeSelected = Value; }
    static void Internal(UCommonButton* Button, UCommonButtonInternal* Value) { Button->NativeInternalButton = Value; }
    static bool HasInternal(UCommonButton* Button) { return Button->NativeInternalButton.IsValid(); }
};

namespace ButtonTextStyleDiagnostic
{
int32 Sequence = 0;
bool Failed = false;
bool Registered = false;
double Deadline = 0;
FDelegateHandle FrameHandle;

void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("BTTEXT v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
}

bool FunctionMetadata(UFunction* F, bool ClassReturn)
{
    if (!F || F->GetOuterUClass() != UCommonButton::StaticClass() || uint32(F->FunctionFlags) != 0x54020401
        || !F->GetNativeFunc() || F->NumParms != 1) return false;
    FProperty* P = F->GetReturnProperty();
    if (!P || P->GetOffset_ForInternal() != 0 || !P->HasAllPropertyFlags(CPF_Parm | CPF_OutParm | CPF_ReturnParm)) return false;
    if (ClassReturn) {
        FClassProperty* C = CastField<FClassProperty>(P);
        return F->ParmsSize == sizeof(UClass*) && C && C->PropertyClass == UClass::StaticClass()
            && C->MetaClass == UCommonTextStyle::StaticClass();
    }
    FBoolProperty* B = CastField<FBoolProperty>(P);
    return F->ParmsSize == sizeof(bool) && B && B->IsNativeBool();
}

bool CheckSelection(UCommonButton* W, UFunction* F, bool Expected)
{
    struct { bool ReturnValue; } Params = {!Expected};
    if (F) W->ProcessEvent(F, &Params);
    return W->GetSelected() == Expected && F && Params.ReturnValue == Expected;
}

bool CheckStyle(UButtonTextProbeWidget* W, UButtonTextProbeInternal* I, UFunction* F, UClass* Expected, int32 Reads, bool First, bool Later)
{
    I->Hover(First, Later);
    UClass* Direct = W->GetCurrentTextStyleClass().Get();
    const int32 DirectReads = I->HoverReads;
    I->Hover(First, Later);
    struct { UClass* ReturnValue; } Params = {nullptr};
    if (F) W->ProcessEvent(F, &Params);
    return F && Direct == Expected && Params.ReturnValue == Expected && DirectReads == Reads && I->HoverReads == Reads;
}

void Run(APlayerController* PC, UWorld* World)
{
    Record(TEXT("begin"), true);
    TStrongObjectPtr<UButtonTextProbeWidget> A(CreateWidget<UButtonTextProbeWidget>(PC, UButtonTextProbeWidget::StaticClass()));
    TStrongObjectPtr<UButtonTextProbeWidget> B(CreateWidget<UButtonTextProbeWidget>(PC, UButtonTextProbeWidget::StaticClass()));
    UButtonTextProbeWidget* CDO = GetMutableDefault<UButtonTextProbeWidget>();
    UCommonButton* BaseCDO = GetMutableDefault<UCommonButton>();
    UButtonTextProbeStyle* S = GetMutableDefault<UButtonTextProbeStyle>();
    Record(TEXT("context"), A.IsValid() && B.IsValid() && CDO && S && A->GetWorld() == World && B->GetWorld() == World);
    if (!A.IsValid() || !B.IsValid() || !CDO || !S) return;
    Record(TEXT("hover_default"), BaseCDO && !A->IsHovered() && !B->IsHovered() && !CDO->IsHovered() && !BaseCDO->IsHovered());
    UFunction* SelectedFn = A->FindFunction(TEXT("GetSelected"));
    UFunction* StyleFn = A->FindFunction(TEXT("GetCurrentTextStyleClass"));
    const bool SelectedMeta = FunctionMetadata(SelectedFn, false);
    const bool StyleMeta = FunctionMetadata(StyleFn, true);
    Record(TEXT("metadata_selected"), SelectedMeta);
    Record(TEXT("metadata_style"), StyleMeta);
    Record(TEXT("native_state_not_reflected"), !FindFProperty<FProperty>(UCommonButton::StaticClass(), TEXT("bNativeSelected"))
        && !FindFProperty<FProperty>(UCommonButton::StaticClass(), TEXT("NativeInternalButton")));
    Record(TEXT("defaults"), SelectedMeta && CheckSelection(CDO, SelectedFn, false) && BaseCDO && CheckSelection(BaseCDO, SelectedFn, false)
        && CheckSelection(A.Get(), SelectedFn, false) && CheckSelection(B.Get(), SelectedFn, false));
    FCommonButtonTextStyleProbeAccess::Set(A.Get(), true);
    Record(TEXT("independent_true"), SelectedMeta && CheckSelection(A.Get(), SelectedFn, true)
        && CheckSelection(B.Get(), SelectedFn, false) && CheckSelection(CDO, SelectedFn, false));
    FCommonButtonTextStyleProbeAccess::Set(B.Get(), true);
    FCommonButtonTextStyleProbeAccess::Set(A.Get(), false);
    Record(TEXT("independent_flip"), SelectedMeta && CheckSelection(A.Get(), SelectedFn, false)
        && CheckSelection(B.Get(), SelectedFn, true));

    TStrongObjectPtr<UButtonTextProbeInternal> Inner(NewObject<UButtonTextProbeInternal>());
    FCommonButtonTextStyleProbeAccess::Internal(A.Get(), Inner.Get());
    const auto OldNormal = S->NormalTextStyle; const auto OldNormalHover = S->NormalHoveredTextStyle;
    const auto OldSelected = S->SelectedTextStyle; const auto OldSelectedHover = S->SelectedHoveredTextStyle;
    const auto OldDisabled = S->DisabledTextStyle;
    const auto OldStyle = A->Style; const bool OldEnabled = A->GetIsEnabled();
    S->NormalTextStyle = UButtonTextProbeNormal::StaticClass();
    S->NormalHoveredTextStyle = UButtonTextProbeNormalHover::StaticClass();
    S->SelectedTextStyle = UButtonTextProbeSelected::StaticClass();
    S->SelectedHoveredTextStyle = UButtonTextProbeSelectedHover::StaticClass();
    S->DisabledTextStyle = UButtonTextProbeDisabled::StaticClass();
    A->Style = UButtonTextProbeStyle::StaticClass();
    A->SetIsEnabled(true);
    Record(TEXT("normal"), StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeNormal::StaticClass(), 1, false, false));
    Record(TEXT("normal_hover"), StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeNormalHover::StaticClass(), 1, true, true));
    FCommonButtonTextStyleProbeAccess::Set(A.Get(), true);
    Record(TEXT("selected"), StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeSelected::StaticClass(), 1, false, false));
    Record(TEXT("selected_hover"), StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeSelectedHover::StaticClass(), 1, true, true));
    A->SetIsEnabled(false);
    Record(TEXT("disabled_precedence"), StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeDisabled::StaticClass(), 0, true, true));
    S->DisabledTextStyle = nullptr;
    Record(TEXT("disabled_fallback"), StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeNormalHover::StaticClass(), 1, true, true));
    A->SetIsEnabled(true);
    S->SelectedHoveredTextStyle = nullptr;
    Record(TEXT("selected_hover_fallback"), StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeSelected::StaticClass(), 1, true, true));
    S->SelectedTextStyle = nullptr;
    Record(TEXT("repeated_hover"), StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeNormalHover::StaticClass(), 2, false, true));
    S->NormalHoveredTextStyle = nullptr;
    Record(TEXT("normal_fallback"), StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeNormal::StaticClass(), 2, true, true));
    S->NormalTextStyle = nullptr;
    Record(TEXT("null_candidates"), StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, nullptr, 2, true, true));
    A->Style = nullptr;
    Record(TEXT("null_style"), StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, nullptr, 0, true, true));
    // Write invalid classes through the real FClassProperty to test runtime validation;
    // TSubclassOf rejects incompatible classes when read.
    FClassProperty* StyleProperty = FindFProperty<FClassProperty>(UCommonButton::StaticClass(), TEXT("Style"));
    FClassProperty* NormalProperty = FindFProperty<FClassProperty>(UCommonButtonStyle::StaticClass(), TEXT("NormalTextStyle"));
    if (StyleProperty) StyleProperty->SetObjectPropertyValue_InContainer(A.Get(), UObject::StaticClass());
    Record(TEXT("invalid_style"), StyleProperty && StyleProperty->GetObjectPropertyValue_InContainer(A.Get()) == UObject::StaticClass() && StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, nullptr, 0, true, true));
    A->Style = UButtonTextProbeStyle::StaticClass();
    if (NormalProperty) NormalProperty->SetObjectPropertyValue_InContainer(S, UObject::StaticClass());
    Record(TEXT("invalid_candidate"), NormalProperty && NormalProperty->GetObjectPropertyValue_InContainer(S) == UObject::StaticClass() && StyleMeta && CheckStyle(A.Get(), Inner.Get(), StyleFn, nullptr, 2, true, true));

    // A transient linked class can have a valid ancestry before a CDO exists.
    // No existing class CDO is removed or replaced by these fixtures.
    TStrongObjectPtr<UClass> MissingOwner(NewObject<UClass>(GetTransientPackage()));
    TStrongObjectPtr<UClass> MissingText(NewObject<UClass>(GetTransientPackage()));
    MissingOwner->SetSuperStruct(UCommonButtonStyle::StaticClass());
    MissingText->SetSuperStruct(UCommonTextStyle::StaticClass());
    MissingOwner->Bind(); MissingOwner->StaticLink(true);
    MissingText->Bind(); MissingText->StaticLink(true);
    A->Style = MissingOwner.Get();
    Record(TEXT("missing_owner_cdo"), !MissingOwner->GetDefaultObject(false) && StyleMeta
        && CheckStyle(A.Get(), Inner.Get(), StyleFn, nullptr, 0, true, true)
        && !MissingOwner->GetDefaultObject(false));
    A->Style = UButtonTextProbeStyle::StaticClass(); A->SetIsEnabled(true);
    FCommonButtonTextStyleProbeAccess::Set(A.Get(), false);
    S->NormalTextStyle = MissingText.Get(); S->NormalHoveredTextStyle = nullptr;
    Record(TEXT("missing_normal_cdo"), !MissingText->GetDefaultObject(false) && StyleMeta
        && CheckStyle(A.Get(), Inner.Get(), StyleFn, nullptr, 1, false, false)
        && !MissingText->GetDefaultObject(false));
    S->NormalTextStyle = UButtonTextProbeNormal::StaticClass();
    S->NormalHoveredTextStyle = MissingText.Get();
    Record(TEXT("missing_hover_cdo"), StyleMeta
        && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeNormal::StaticClass(), 1, true, true)
        && !MissingText->GetDefaultObject(false));
    FCommonButtonTextStyleProbeAccess::Set(A.Get(), true);
    S->SelectedTextStyle = MissingText.Get(); S->NormalHoveredTextStyle = nullptr;
    Record(TEXT("missing_selected_cdo"), StyleMeta
        && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeNormal::StaticClass(), 2, false, false)
        && !MissingText->GetDefaultObject(false));
    S->SelectedTextStyle = UButtonTextProbeSelected::StaticClass();
    S->SelectedHoveredTextStyle = MissingText.Get();
    Record(TEXT("missing_selected_hover_cdo"), StyleMeta
        && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeSelected::StaticClass(), 1, true, true)
        && !MissingText->GetDefaultObject(false));
    A->SetIsEnabled(false); S->DisabledTextStyle = MissingText.Get();
    Record(TEXT("missing_disabled_cdo"), StyleMeta
        && CheckStyle(A.Get(), Inner.Get(), StyleFn, UButtonTextProbeNormal::StaticClass(), 1, false, false)
        && !MissingText->GetDefaultObject(false));

    // Load the original button class used by the known viewport failure. Its
    // serialized Style must resolve; no fixture class substitutes for it.
    UClass* AssetButtonClass = LoadClass<UCommonButton>(nullptr, TEXT("/Game/UI/Foundation/Buttons/IconTextButton.IconTextButton_C"));
    UCommonButton* AssetButtonCDO = AssetButtonClass ? Cast<UCommonButton>(AssetButtonClass->GetDefaultObject()) : nullptr;
    UClass* AssetStyle = AssetButtonCDO ? AssetButtonCDO->Style.Get() : nullptr;
    bool AssetOK = false;
    if (AssetStyle && AssetStyle->HasAnyFlags(RF_WasLoaded)) {
        A->Style = AssetStyle; A->SetIsEnabled(true); FCommonButtonTextStyleProbeAccess::Set(A.Get(), false);
        UCommonButtonStyle* AssetCDO = Cast<UCommonButtonStyle>(AssetStyle->GetDefaultObject());
        UClass* Normal = AssetCDO ? AssetCDO->NormalTextStyle.Get() : nullptr;
        UCommonTextStyle* TextCDO = Normal ? Cast<UCommonTextStyle>(Normal->GetDefaultObject()) : nullptr;
        AssetOK = StyleMeta && AssetCDO && TextCDO && CheckStyle(A.Get(), Inner.Get(), StyleFn, TextCDO->GetClass(), 1, false, false);
        UE_LOG(LogTemp, Display, TEXT("BUTTON_STYLE_ASSET button=%s style=%s text=%s"),
            *AssetButtonClass->GetPathName(), *AssetStyle->GetPathName(), *GetPathNameSafe(TextCDO));
    }
    Failed |= !AssetOK;
    UE_LOG(LogTemp, Display, TEXT("BTTEXT v=1 seq=%d event=loaded_asset available=%d ok=%d"), ++Sequence, int32(AssetStyle != nullptr), int32(AssetOK));

    S->NormalTextStyle = OldNormal; S->NormalHoveredTextStyle = OldNormalHover;
    S->SelectedTextStyle = OldSelected; S->SelectedHoveredTextStyle = OldSelectedHover; S->DisabledTextStyle = OldDisabled;
    A->Style = OldStyle; A->SetIsEnabled(OldEnabled);
    FCommonButtonTextStyleProbeAccess::Set(A.Get(), false); FCommonButtonTextStyleProbeAccess::Set(B.Get(), false);
    Inner.Reset(); CollectGarbage(RF_NoFlags, true);
    Record(TEXT("hover_weak_expired"), !FCommonButtonTextStyleProbeAccess::HasInternal(A.Get()) && !A->IsHovered());
    Record(TEXT("restored"), S->NormalTextStyle == OldNormal && S->NormalHoveredTextStyle == OldNormalHover
        && S->SelectedTextStyle == OldSelected && S->SelectedHoveredTextStyle == OldSelectedHover && S->DisabledTextStyle == OldDisabled
        && A->Style == OldStyle && A->GetIsEnabled() == OldEnabled && !A->GetSelected() && !B->GetSelected() && !CDO->GetSelected() && BaseCDO && !BaseCDO->GetSelected());
}

void Finish()
{
    FCoreDelegates::OnEndFrame.Remove(FrameHandle);
    Record(TEXT("complete"), !Failed);
    FPlatformMisc::RequestExitWithStatus(false, Failed ? 1 : 0);
}
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

void RegisterCommonButtonTextStyleProbeDriver()
{
    using namespace ButtonTextStyleDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("CommonButtonTextStyleProbe"))) return;
    Registered = true; Deadline = FPlatformTime::Seconds() + 180.0;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
