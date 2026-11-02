#include "UCommonInputSubsystem.h"
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
#include "UObject/UnrealType.h"

namespace CommonInputCurrentTypeDiagnostic
{
int32 Sequence = 0;
bool Failed = false;
bool Registered = false;
double Deadline = 0;
FDelegateHandle FrameHandle;

void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("INPUTTYPE v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
}

void Run(ULocalPlayer* Player)
{
    Record(TEXT("begin"), true);
    UCommonInputSubsystem* Input = Player->GetSubsystem<UCommonInputSubsystem>();
    Record(TEXT("context"), Input && Input->GetLocalPlayer() == Player && Input->GetOuter() == Player);
    if (!Input) return;

    UFunction* Function = Input->FindFunction(FName(TEXT("GetCurrentInputType")));
    FEnumProperty* Return = Function ? CastField<FEnumProperty>(Function->GetReturnProperty()) : nullptr;
    const bool ValidFunction = Function && Function->GetOuterUClass() == UCommonInputSubsystem::StaticClass()
        && uint32(Function->FunctionFlags) == 0x54020401 && Function->GetNativeFunc()
        && Function->NumParms == 1 && Function->ParmsSize == 1 && Return
        && Return->GetEnum() == StaticEnum<ECommonInputType>()
        && CastField<FByteProperty>(Return->GetUnderlyingProperty())
        && Return->GetOffset_ForInternal() == 0 && Return->HasAllPropertyFlags(CPF_Parm | CPF_OutParm | CPF_ReturnParm);
    Record(TEXT("metadata"), ValidFunction);

    const ECommonInputType Saved = Input->CurrentInputType;
    const ECommonInputType SavedLast = Input->LastInputType;
    const ECommonGamepadType SavedGamepad = Input->GamepadInputType;
    const bool SavedSimulated = Input->bIsGamepadSimulatedClick;
    // Deliberately exercise the mutable field, including repeated and unnamed
    // byte values. This diagnostic does not synthesize input events or producers.
    const uint8 Values[] = {0, 2, 1, 0, 3, 255, 1, 2};
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Values); ++Index)
    {
        const uint8 Value = Values[Index];
        Input->CurrentInputType = static_cast<ECommonInputType>(Value);
        const UCommonInputSubsystem* ConstInput = Input;
        const uint8 Direct = static_cast<uint8>(ConstInput->GetCurrentInputType());
        struct { ECommonInputType ReturnValue; } Parameters = {static_cast<ECommonInputType>(Value ^ 0xff)};
        static_assert(sizeof(Parameters) == 1, "Original accessor has a one-byte return block");
        if (ValidFunction) Input->ProcessEvent(Function, &Parameters);
        const uint8 Reflected = static_cast<uint8>(Parameters.ReturnValue);
        const bool OK = ValidFunction && Direct == Value && Reflected == Value
            && static_cast<uint8>(Input->CurrentInputType) == Value && Input->LastInputType == SavedLast
            && Input->GamepadInputType == SavedGamepad && Input->bIsGamepadSimulatedClick == SavedSimulated;
        Failed |= !OK;
        UE_LOG(LogTemp, Display, TEXT("INPUTTYPE v=1 seq=%d event=state index=%d value=%d direct=%d reflected=%d ok=%d"),
            ++Sequence, Index, int32(Value), int32(Direct), int32(Reflected), int32(OK));
    }
    Input->CurrentInputType = Saved;
    Record(TEXT("restored"), Input->CurrentInputType == Saved && Input->GetCurrentInputType() == Saved);
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
    for (const FWorldContext& Context : GEngine->GetWorldContexts())
    {
        UWorld* World = Context.World();
        UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
        ULocalPlayer* Player = GI ? GI->GetFirstGamePlayer() : nullptr;
        APlayerController* PC = Player ? Player->GetPlayerController(World) : nullptr;
        if (!World || !World->IsGameWorld() || !GI || !Player || !PC || PC->Player != Player
            || PC->GetWorld() != World || Player->GetGameInstance() != GI || GI->GetWorld() != World) continue;
        Run(Player); Finish(); return;
    }
}
}

void RegisterCommonInputCurrentTypeProbeDriver()
{
    using namespace CommonInputCurrentTypeDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("CommonInputCurrentTypeProbe"))) return;
    Registered = true;
    Deadline = FPlatformTime::Seconds() + 180.0;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
