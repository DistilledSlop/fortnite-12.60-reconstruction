#include "BaseClickProbe.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"

namespace BaseClickDiagnostic
{
int32 Sequence = 0; bool Failed = false, Registered = false; double Deadline = 0; FDelegateHandle FrameHandle;
void Record(const TCHAR* Event, bool OK)
{
    Failed |= !OK;
    UE_LOG(LogTemp, Display, TEXT("BTCLICK v=1 seq=%d event=%s ok=%d"), ++Sequence, Event, int32(OK));
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
        if (!World || !World->IsGameWorld() || !World->GetGameInstance()) continue;
        Record(TEXT("begin"), true);
        const bool Ready = IsInGameThread() && FSlateApplication::IsInitialized();
        Record(TEXT("context"), Ready);
        if (Ready) Failed |= !RunBaseClickProbe(World, [](const TCHAR* Event, bool OK) { Record(Event, OK); });
        Finish(); return;
    }
}
}
void RegisterBaseClickProbeDriver()
{
    using namespace BaseClickDiagnostic;
    if (Registered || !FParse::Param(FCommandLine::Get(), TEXT("ButtonBaseClickProbe"))) return;
    Registered = true; Deadline = FPlatformTime::Seconds() + 180;
    FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&Tick);
}
