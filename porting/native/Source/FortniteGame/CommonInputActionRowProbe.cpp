#include "CommonInputActionRowProbe.h"
#include "FCommonInputActionData.h"
#include "FCommonInputPlatformData.h"
#include "UCommonInputSettings.h"
#include "CommonInputModuleRuntime.h"
#include "CommonInputControllerBrushLookup.h"
#include "Engine/Texture2D.h"
#include "Engine/DataTable.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "UObject/Class.h"
#include "UObject/UnrealType.h"

namespace CommonInputActionRowProbe
{
bool Failed = false;
bool Registered = false;
FDelegateHandle FrameHandle;

void Record(const TCHAR* Row, bool Passed)
{
    Failed |= !Passed;
    UE_LOG(LogTemp, Display, TEXT("COMMON_INPUT_ACTION_ROW_PROBE row=%s passed=%d"), Row, int32(Passed));
}

template <typename T, typename M> int64 Offset(const T& Object, const M& Member)
{
    return reinterpret_cast<const uint8*>(&Member) - reinterpret_cast<const uint8*>(&Object);
}

bool InfoDefaults(const FCommonInputTypeInfo& Info)
{
    FSlateBrush Brush;
    Brush.DrawAs = ESlateBrushDrawType::NoDrawType;
    return Info.Key == FKey() && Info.OverrrideState == EInputActionState::Enabled &&
        !Info.bActionRequiresHold && Info.HoldTime == 0.5f && Info.OverrideBrush == Brush;
}

void CheckDefaults(const TCHAR* Prefix, const FCommonInputActionData& Row)
{
    Record(*FString::Printf(TEXT("%s_text"), Prefix), Row.DisplayName.IsEmpty() && Row.HoldDisplayName.IsEmpty());
    Record(*FString::Printf(TEXT("%s_keyboard"), Prefix), InfoDefaults(Row.KeyboardInputTypeInfo));
    for (int32 Index = 0; Index < 4; ++Index)
        Record(*FString::Printf(TEXT("%s_gamepad%d"), Prefix, Index), InfoDefaults(Row.GamepadInputTypeInfos[Index]));
    Record(*FString::Printf(TEXT("%s_touch"), Prefix), InfoDefaults(Row.TouchInputTypeInfo));
}

bool HasCopy(UScriptStruct* Type)
{
    return Type && Type->GetCppStructOps() && Type->GetCppStructOps()->HasCopy();
}

bool PlatformDefaults(const FCommonInputPlatformData& Value)
{
    return !Value.bSupported && Value.DefaultInputType == ECommonInputType::Gamepad &&
        !Value.bSupportsMouseAndKeyboard && Value.bSupportsGamepad &&
        Value.DefaultGamepadType == ECommonGamepadType::XboxOneController &&
        Value.bCanChangeGamepadType && !Value.bSupportsTouch &&
        Value.ControllerData.Num() == 0 && Value.ControllerDataClasses.Num() == 0;
}

bool SamePlatform(const FCommonInputPlatformData& A, const FCommonInputPlatformData& B)
{
    return A.bSupported == B.bSupported && A.DefaultInputType == B.DefaultInputType &&
        A.bSupportsMouseAndKeyboard == B.bSupportsMouseAndKeyboard && A.bSupportsGamepad == B.bSupportsGamepad &&
        A.DefaultGamepadType == B.DefaultGamepadType && A.bCanChangeGamepadType == B.bCanChangeGamepadType &&
        A.bSupportsTouch == B.bSupportsTouch && A.ControllerData == B.ControllerData &&
        A.ControllerDataClasses == B.ControllerDataClasses;
}

void CheckBatch()
{
    UScriptStruct* Type = FCommonInputPlatformData::StaticStruct();
    void* Storage = FMemory::Malloc(Type->GetStructureSize(), Type->GetMinAlignment());
    FMemory::Memset(Storage, 0xa5, Type->GetStructureSize());
    Type->InitializeStruct(Storage);
    Record(TEXT("batch_platform_initialize"), PlatformDefaults(*static_cast<FCommonInputPlatformData*>(Storage)));
    Type->DestroyStruct(Storage);
    FMemory::Free(Storage);

    const UCommonInputSettings* Defaults = GetDefault<UCommonInputSettings>();
    UCommonInputSettings* Fresh = NewObject<UCommonInputSettings>();
    bool bSettingsMatch = Defaults && Fresh;
    if (bSettingsMatch)
    {
        for (int32 I = 0; I < 7; ++I)
        {
            const FCommonInputPlatformData& P = Defaults->CommonInputPlatformData[I];
            UE_LOG(LogTemp, Display, TEXT("COMMON_INPUT_BATCH_SETTINGS index=%d supported=%d input=%d mouse=%d gamepad=%d padtype=%d change=%d touch=%d soft=%d classes=%d"),
                I, int32(P.bSupported), int32(P.DefaultInputType), int32(P.bSupportsMouseAndKeyboard),
                int32(P.bSupportsGamepad), int32(P.DefaultGamepadType), int32(P.bCanChangeGamepadType),
                int32(P.bSupportsTouch), P.ControllerData.Num(), P.ControllerDataClasses.Num());
            bSettingsMatch &= SamePlatform(P, Fresh->CommonInputPlatformData[I]);
        }
        UE_LOG(LogTemp, Display, TEXT("COMMON_INPUT_BATCH_SETTINGS thrashing=%d limit=%d window=%g cooldown=%g"),
            int32(Defaults->bEnableInputMethodThrashingProtection), Defaults->InputMethodThrashingLimit,
            Defaults->InputMethodThrashingWindowInSeconds, Defaults->InputMethodThrashingCooldownInSeconds);
        bSettingsMatch &= Defaults->bEnableInputMethodThrashingProtection == Fresh->bEnableInputMethodThrashingProtection &&
            Defaults->InputMethodThrashingLimit == Fresh->InputMethodThrashingLimit &&
            Defaults->InputMethodThrashingWindowInSeconds == Fresh->InputMethodThrashingWindowInSeconds &&
            Defaults->InputMethodThrashingCooldownInSeconds == Fresh->InputMethodThrashingCooldownInSeconds;
    }
    Record(TEXT("batch_settings_cdo_fresh"), bSettingsMatch);

    UCommonInputProbeControllerFirst* First = GetMutableDefault<UCommonInputProbeControllerFirst>();
    UCommonInputProbeControllerSecond* Second = GetMutableDefault<UCommonInputProbeControllerSecond>();
    First->InputType = Second->InputType = ECommonInputType::Gamepad;
    First->GamepadType = Second->GamepadType = ECommonGamepadType::PS4Controller;
    FCommonInputKeyBrushConfiguration Entry;
    Entry.Key = EKeys::A;
    UTexture2D* Texture = NewObject<UTexture2D>();
    Entry.KeyBrush.SetResourceObject(Texture);
    Entry.KeyBrush.ImageSize = FVector2D(37.f, 19.f);
    Second->InputBrushDataMap.Add(Entry);
    FSlateBrush Out;
    FCommonInputPlatformData Platform;
    Platform.ControllerDataClasses.Add(UCommonInputProbeControllerFirst::StaticClass());
    Platform.ControllerDataClasses.Add(UCommonInputProbeControllerSecond::StaticClass());
    TArray<FKey> Single; Single.Add(EKeys::A);
    Record(TEXT("batch_first_match_stops"), !FindCommonInputPlatformBrush(Platform, Single,
        ECommonInputType::Gamepad, ECommonGamepadType::PS4Controller, Out));
    First->GamepadType = ECommonGamepadType::XboxOneController;
    bool bFound = FindCommonInputPlatformBrush(Platform, Single, ECommonInputType::Gamepad,
        ECommonGamepadType::PS4Controller, Out);
    Record(TEXT("batch_brush_copy"), bFound && Out.GetResourceObject() == Texture && Out.ImageSize == FVector2D(37.f, 19.f));
    FCommonInputKeySetBrushConfiguration Set;
    Set.Keys.Add(EKeys::A); Set.Keys.Add(EKeys::B);
    Set.KeyBrush = Entry.KeyBrush;
    First->InputBrushKeySets.Add(Set);
    TArray<FKey> Duplicates; Duplicates.Add(EKeys::A); Duplicates.Add(EKeys::A);
    Out = FSlateBrush();
    bFound = FindCommonInputControllerBrush(*First, Duplicates, Out);
    Record(TEXT("batch_keyset_nonconsuming"), bFound && Out.GetResourceObject() == Texture && Out.ImageSize == FVector2D(37.f, 19.f));
    Duplicates.Add(EKeys::A);
    Record(TEXT("batch_keyset_exact_count"), !FindCommonInputControllerBrush(*First, Duplicates, Out));
    First->InputBrushKeySets.Empty();
    Second->InputBrushDataMap.Empty();
}

void CheckCacheLifecycle()
{
    UCommonInputSettings* ModuleSettings = GetCommonInputSettings();
    Record(TEXT("cache_module_instance"), ModuleSettings && ModuleSettings != GetDefault<UCommonInputSettings>() &&
        ModuleSettings->GetOuter() == GetTransientPackage() && ModuleSettings->IsRooted() &&
        GetCommonInputSettings() == ModuleSettings);
    UCommonInputSettings* Settings = NewObject<UCommonInputSettings>();
    Settings->InputData.Reset();
    Settings->CommonInputPlatformData[0].ControllerData.Empty();
    Settings->CommonInputPlatformData[0].ControllerDataClasses.Empty();
    Settings->CommonInputPlatformData[0].ControllerData.Add(UCommonInputProbeControllerFirst::StaticClass());
    FCommonInputPlatformData Copy = Settings->NativeGetCurrentPlatformData();
    Record(TEXT("cache_resolve_and_copy"), Copy.ControllerDataClasses.Num() == 1 &&
        Copy.ControllerDataClasses[0] == UCommonInputProbeControllerFirst::StaticClass() &&
        Settings->CurrentPlatform.ControllerDataClasses.Num() == 1 && !Settings->InputDataClass);
    Copy.ControllerDataClasses.Empty();
    Record(TEXT("cache_return_by_value"), Settings->CurrentPlatform.ControllerDataClasses.Num() == 1);
    Settings->CurrentPlatform.ControllerDataClasses.Empty();
    Copy = Settings->NativeGetCurrentPlatformData();
    Record(TEXT("cache_membership_invalidation"), Copy.ControllerDataClasses.Num() == 1 &&
        Copy.ControllerDataClasses[0] == UCommonInputProbeControllerFirst::StaticClass());
}

void Run()
{
    FCommonInputActionData Row;
    const FCommonInputTypeInfo& Info = Row.KeyboardInputTypeInfo;
    UScriptStruct* RowStruct = FCommonInputActionData::StaticStruct();
    FStructProperty* InfoProperty = RowStruct ? FindFProperty<FStructProperty>(RowStruct, TEXT("KeyboardInputTypeInfo")) : nullptr;
    UScriptStruct* InfoStruct = InfoProperty ? InfoProperty->Struct : nullptr;
    UE_LOG(LogTemp, Display, TEXT("COMMON_INPUT_ACTION_ROW_PROBE row=layout_values row_size=%d row_align=%d display=%lld hold_display=%lld keyboard=%lld gamepad=%lld touch=%lld info_size=%d info_align=%d key=%lld state=%lld requires_hold=%lld hold_time=%lld brush=%lld"),
        int32(sizeof(Row)), int32(alignof(FCommonInputActionData)), (long long)Offset(Row, Row.DisplayName),
        (long long)Offset(Row, Row.HoldDisplayName), (long long)Offset(Row, Row.KeyboardInputTypeInfo),
        (long long)Offset(Row, Row.GamepadInputTypeInfos), (long long)Offset(Row, Row.TouchInputTypeInfo),
        int32(sizeof(Info)), int32(alignof(FCommonInputTypeInfo)), (long long)Offset(Info, Info.Key),
        (long long)Offset(Info, Info.OverrrideState), (long long)Offset(Info, Info.bActionRequiresHold),
        (long long)Offset(Info, Info.HoldTime), (long long)Offset(Info, Info.OverrideBrush));
    Record(TEXT("row_layout"), sizeof(Row) == 1064 && alignof(FCommonInputActionData) == 8 &&
        Offset(Row, Row.DisplayName) == 8 && Offset(Row, Row.HoldDisplayName) == 32 &&
        Offset(Row, Row.KeyboardInputTypeInfo) == 56 && Offset(Row, Row.GamepadInputTypeInfos) == 224 &&
        Offset(Row, Row.TouchInputTypeInfo) == 896 && RowStruct && RowStruct->GetStructureSize() == 1064 &&
        RowStruct->GetMinAlignment() == 8 && RowStruct->IsChildOf(FTableRowBase::StaticStruct()));
    Record(TEXT("info_layout"), sizeof(Info) == 168 && alignof(FCommonInputTypeInfo) == 8 &&
        Offset(Info, Info.Key) == 0 && Offset(Info, Info.OverrrideState) == 24 &&
        Offset(Info, Info.bActionRequiresHold) == 25 && Offset(Info, Info.HoldTime) == 28 &&
        Offset(Info, Info.OverrideBrush) == 32 && InfoStruct && InfoStruct->GetStructureSize() == 168 &&
        InfoStruct->GetMinAlignment() == 8);
    CheckDefaults(TEXT("cpp"), Row);
    Record(TEXT("row_struct_ops_copy"), HasCopy(RowStruct));
    Record(TEXT("info_struct_ops_copy"), HasCopy(InfoStruct));
    if (RowStruct)
    {
        void* Storage = FMemory::Malloc(RowStruct->GetStructureSize(), RowStruct->GetMinAlignment());
        RowStruct->InitializeStruct(Storage);
        CheckDefaults(TEXT("script"), *static_cast<FCommonInputActionData*>(Storage));
        RowStruct->DestroyStruct(Storage);
        FMemory::Free(Storage);
    }
    else Record(TEXT("script_initialize"), false);
    if (InfoStruct)
    {
        void* Storage = FMemory::Malloc(InfoStruct->GetStructureSize(), InfoStruct->GetMinAlignment());
        InfoStruct->InitializeStruct(Storage);
        Record(TEXT("info_script_initialize"), InfoDefaults(*static_cast<FCommonInputTypeInfo*>(Storage)));
        InfoStruct->DestroyStruct(Storage);
        FMemory::Free(Storage);
    }
    else Record(TEXT("info_script_initialize"), false);

    UDataTable* Table = LoadObject<UDataTable>(nullptr, TEXT("/Game/UI/Foundation/Input/InputActionDataTable.InputActionDataTable"));
    Record(TEXT("asset_load"), Table != nullptr);
    Record(TEXT("asset_row_struct"), Table && RowStruct && Table->GetRowStruct() == RowStruct);
    Record(TEXT("asset_nonempty"), Table && Table->GetRowMap().Num() > 0);
    if (Table)
    {
        TArray<FString> Names;
        for (const auto& Entry : Table->GetRowMap()) Names.Add(Entry.Key.ToString());
        Names.Sort();
        UE_LOG(LogTemp, Display, TEXT("COMMON_INPUT_ACTION_ROW_PROBE row=asset_values row_count=%d"), Names.Num());
        for (int32 Index = 0; Index < FMath::Min(5, Names.Num()); ++Index)
            UE_LOG(LogTemp, Display, TEXT("COMMON_INPUT_ACTION_ROW_PROBE row=asset_sample index=%d name=%s"), Index, *Names[Index]);
    }
}

void Tick()
{
    FCoreDelegates::OnEndFrame.Remove(FrameHandle);
    Run();
    CheckBatch();
    CheckCacheLifecycle();
    Record(TEXT("complete"), !Failed);
    FPlatformMisc::RequestExitWithStatus(false, Failed ? 1 : 0);
}
}

void RegisterCommonInputActionRowProbeDriver()
{
    if (CommonInputActionRowProbe::Registered || !FParse::Param(FCommandLine::Get(), TEXT("CommonInputActionRowProbe"))) return;
    CommonInputActionRowProbe::Registered = true;
    CommonInputActionRowProbe::FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&CommonInputActionRowProbe::Tick);
}
