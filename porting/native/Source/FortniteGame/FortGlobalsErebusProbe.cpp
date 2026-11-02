#include "UFortGlobals.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "UObject/Stack.h"
#include "UObject/UnrealType.h"

namespace ErebusDiagnostic
{
int32 Sequence = 0;
bool Failed = false, Registered = false;
double Deadline = 0;
FDelegateHandle FrameHandle;
void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("EREBUS v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
}
void Run()
{
    Record(TEXT("begin"), true);
    UFortGlobals* CDO = GetMutableDefault<UFortGlobals>();
    UFunction* Fn = UFortGlobals::StaticClass()->FindFunctionByName(TEXT("IsErebus"));
    FBoolProperty* Return = Fn ? CastField<FBoolProperty>(Fn->GetReturnProperty()) : nullptr;
    // The original metadata is declaration flags; FBoolProperty adds these traits.
    const EPropertyFlags ReturnFlags = EPropertyFlags(0x10000000000580ULL)
        | CPF_IsPlainOldData | CPF_NoDestructor | CPF_ZeroConstructor | CPF_HasGetValueTypeHash;
    const bool Meta = Fn && Fn->GetOuterUClass() == UFortGlobals::StaticClass()
        && uint32(Fn->FunctionFlags) == 0x14022401 && Fn->GetNativeFunc()
        && Fn->NumParms == 1 && Fn->ParmsSize == sizeof(bool) && Fn->ReturnValueOffset == 0
        && Return && Return->GetFName() == FName(TEXT("ReturnValue"))
        && Return->GetOffset_ForInternal() == 0 && Return->IsNativeBool()
        && Return->GetPropertyFlags() == ReturnFlags;
    Record(TEXT("metadata"), Meta);
    Record(TEXT("cdo"), CDO && CDO->HasAnyFlags(RF_ClassDefaultObject));
    Record(TEXT("direct"), !UFortGlobals::IsErebus());
    struct { bool ReturnValue; } Params = {true};
    if (Meta && CDO) CDO->ProcessEvent(Fn, &Params);
    Record(TEXT("reflected_true"), Meta && CDO && !Params.ReturnValue);
    Params.ReturnValue = false;
    if (Meta && CDO) CDO->ProcessEvent(Fn, &Params);
    Record(TEXT("reflected_false"), Meta && CDO && !Params.ReturnValue);
    bool EndOK = false, NullOK = false;
    if (Meta && CDO)
    {
        // Exercise the actual generated thunk with both original FFrame.Code modes.
        // This is an engine frame invocation, not a synthetic Blueprint program.
        uint8 End = EX_EndFunctionParms;
        FFrame Frame(CDO, Fn, nullptr);
        Frame.Code = &End;
        Params.ReturnValue = true;
        Fn->Invoke(CDO, Frame, &Params.ReturnValue);
        EndOK = !Params.ReturnValue && Frame.Code == &End + 1;
        Frame.Code = nullptr;
        Params.ReturnValue = true;
        Fn->Invoke(CDO, Frame, &Params.ReturnValue);
        NullOK = !Params.ReturnValue && Frame.Code == nullptr;
    }
    Record(TEXT("frame_end_marker"), EndOK);
    Record(TEXT("frame_null_code"), NullOK);
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
        if (Context.World() && Context.World()->IsGameWorld()) { Run(); Finish(); return; }
}
}
void RegisterFortGlobalsErebusProbeDriver()
{
    using namespace ErebusDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("FortGlobalsErebusProbe"))) return;
    Registered = true;
    Deadline = FPlatformTime::Seconds() + 180;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
