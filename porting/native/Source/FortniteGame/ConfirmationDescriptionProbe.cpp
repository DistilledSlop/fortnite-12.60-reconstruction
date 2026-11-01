#include "ConfirmationDescriptionProbe.h"
#include "Generated/FFortDialogDescription_NUI.h"
#include "HAL/PlatformMisc.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "UObject/Class.h"

namespace ConfirmationDescriptionProbe
{
bool Failed = false;
bool Registered = false;
FDelegateHandle FrameHandle;

void Record(const TCHAR* Row, bool Passed)
{
    Failed |= !Passed;
    UE_LOG(LogTemp, Display, TEXT("CONFIRMATION_DESCRIPTION_PROBE row=%s passed=%d"), Row, int32(Passed));
}

bool Defaults(const FFortDialogDescription_NUI& D)
{
    const FSlateBrush DefaultBrush;
    return D.Icon == DefaultBrush && D.MessageHeader.IsEmpty() && D.MessageBody.IsEmpty() &&
        D.ConfirmButtonInputActions.Num() == 0 && D.DeclineButtonInputAction.IsNone() &&
        D.AdditionalContent == nullptr && D.LeftAdditionalContent == nullptr &&
        D.DisplayTime == 0.0f && D.Dismissable && !D.Cancelable &&
        !D.bShouldWaitForLatentActionOnConfirmAction && D.NotificationHandler == nullptr &&
        D.ShowSound == nullptr && !D.NativeResultCallback.IsBound() &&
        !D.NativeAdvancedResultCallback.IsBound();
}

struct FSentinel {};

// Each C++ path is tested with no other delegate copies retaining its capture.
bool CppLifetime(bool Assign)
{
    TSharedPtr<FSentinel> Owner = MakeShareable(new FSentinel());
    TWeakPtr<FSentinel> Weak = Owner;
    int32 Calls = 0;
    bool AliveAfterSource = false;
    {
        FFortDialogDescription_NUI Source;
        Source.NativeResultCallback.BindLambda([Owner, &Calls](EFortDialogResult, FName) { if (Owner.IsValid()) ++Calls; });
        FFortDialogDescription_NUI Copy(Source);
        FFortDialogDescription_NUI Assigned;
        if (Assign) Assigned = Source;
        Source.NativeResultCallback.Unbind();
        Owner.Reset();
        if (Assign) Copy.NativeResultCallback.Unbind();
        FFortDialogDescription_NUI& Survivor = Assign ? Assigned : Copy;
        AliveAfterSource = Weak.IsValid();
        Survivor.NativeResultCallback.ExecuteIfBound(EFortDialogResult::Confirmed, NAME_None);
    }
    return AliveAfterSource && Calls == 1 && !Weak.IsValid();
}

void Run()
{
    FFortDialogDescription_NUI Source;
    UScriptStruct* Struct = FFortDialogDescription_NUI::StaticStruct();
    const auto* Base = reinterpret_cast<const uint8*>(&Source);
    const int64 OrdinaryOffset = reinterpret_cast<const uint8*>(&Source.NativeResultCallback) - Base;
    const int64 AdvancedOffset = reinterpret_cast<const uint8*>(&Source.NativeAdvancedResultCallback) - Base;
    UE_LOG(LogTemp, Display, TEXT("CONFIRMATION_DESCRIPTION_PROBE row=layout_values size=%d align=%d ordinary=%lld advanced=%lld struct_size=%d struct_align=%d"),
        int32(sizeof(Source)), int32(alignof(FFortDialogDescription_NUI)),
        (long long)OrdinaryOffset, (long long)AdvancedOffset,
        Struct ? Struct->GetStructureSize() : -1, Struct ? Struct->GetMinAlignment() : -1);
    Record(TEXT("layout"), Struct && sizeof(Source) == 280 && alignof(FFortDialogDescription_NUI) == 8 &&
        OrdinaryOffset == 248 && AdvancedOffset == 264 && Struct->GetStructureSize() == 280 &&
        Struct->GetMinAlignment() == 8);
    Record(TEXT("cpp_defaults"), Defaults(Source));
    Record(TEXT("cpp_copy_lifetime"), CppLifetime(false));
    Record(TEXT("cpp_assignment_lifetime"), CppLifetime(true));
    const bool HasCopy = Struct && Struct->GetCppStructOps() && Struct->GetCppStructOps()->HasCopy();
    Record(TEXT("struct_ops_copy"), HasCopy);
    if (!Struct || !HasCopy) return;

    void* Storage = FMemory::Malloc(Struct->GetStructureSize(), Struct->GetMinAlignment());
    Struct->InitializeStruct(Storage);
    auto* ScriptCopy = static_cast<FFortDialogDescription_NUI*>(Storage);
    Record(TEXT("struct_initialize_defaults"), Defaults(*ScriptCopy));

    int32 OrdinaryCalls = 0, AdvancedCalls = 0;
    bool ArgumentsCorrect = true;
    TSharedPtr<FSentinel> OrdinaryOwner = MakeShareable(new FSentinel());
    TSharedPtr<FSentinel> AdvancedOwner = MakeShareable(new FSentinel());
    TWeakPtr<FSentinel> OrdinaryWeak = OrdinaryOwner;
    TWeakPtr<FSentinel> AdvancedWeak = AdvancedOwner;
    const FName ExpectedName(TEXT("ConfirmationProbe"));
    Source.NativeResultCallback.BindLambda([OrdinaryOwner, &OrdinaryCalls, &ArgumentsCorrect, ExpectedName](EFortDialogResult Result, FName Name)
    {
        ++OrdinaryCalls;
        ArgumentsCorrect &= OrdinaryOwner.IsValid() && Result == EFortDialogResult::Declined && Name == ExpectedName;
    });
    Source.NativeAdvancedResultCallback.BindLambda([AdvancedOwner, &AdvancedCalls, &ArgumentsCorrect, ExpectedName](EFortDialogResult Result, FName Name, bool Wait, FFortDialogExternalLatentActionHandle Handle)
    {
        ++AdvancedCalls;
        ArgumentsCorrect &= AdvancedOwner.IsValid() && Result == EFortDialogResult::TimedOut &&
            Name == ExpectedName && Wait && Handle.Handle == 0x12345678;
    });
    Source.DisplayTime = 3.25f;
    Source.DeclineButtonInputAction = ExpectedName;
    FFortDialogDescription_NUI CppCopy(Source);
    FFortDialogDescription_NUI CppAssigned;
    CppAssigned = Source;
    Struct->CopyScriptStruct(ScriptCopy, &Source);
    Record(TEXT("copies_bound"), CppCopy.NativeResultCallback.IsBound() && CppCopy.NativeAdvancedResultCallback.IsBound() &&
        CppAssigned.NativeResultCallback.IsBound() && CppAssigned.NativeAdvancedResultCallback.IsBound() &&
        ScriptCopy->NativeResultCallback.IsBound() && ScriptCopy->NativeAdvancedResultCallback.IsBound() &&
        CppCopy.DisplayTime == 3.25f && CppAssigned.DeclineButtonInputAction == ExpectedName &&
        ScriptCopy->DisplayTime == 3.25f && ScriptCopy->DeclineButtonInputAction == ExpectedName);
    Source.NativeResultCallback.Unbind();
    Source.NativeAdvancedResultCallback.Unbind();
    OrdinaryOwner.Reset(); AdvancedOwner.Reset();
    Record(TEXT("copies_keep_owners_alive"), OrdinaryWeak.IsValid() && AdvancedWeak.IsValid());
    FFortDialogExternalLatentActionHandle Handle;
    Handle.Handle = 0x12345678;
    for (FFortDialogDescription_NUI* Copy : { &CppCopy, &CppAssigned, ScriptCopy })
    {
        Copy->NativeResultCallback.ExecuteIfBound(EFortDialogResult::Declined, ExpectedName);
        Copy->NativeAdvancedResultCallback.ExecuteIfBound(EFortDialogResult::TimedOut, ExpectedName, true, Handle);
    }
    Record(TEXT("copied_callbacks_arguments"), ArgumentsCorrect && OrdinaryCalls == 3 && AdvancedCalls == 3);
    CppCopy.NativeResultCallback.Unbind(); CppCopy.NativeAdvancedResultCallback.Unbind();
    CppAssigned.NativeResultCallback.Unbind(); CppAssigned.NativeAdvancedResultCallback.Unbind();
    Record(TEXT("script_copy_keeps_owners_alive"), OrdinaryWeak.IsValid() && AdvancedWeak.IsValid());
    Struct->DestroyStruct(Storage);
    FMemory::Free(Storage);
    Record(TEXT("destroy_releases_owners"), !OrdinaryWeak.IsValid() && !AdvancedWeak.IsValid());
}

void Tick()
{
    FCoreDelegates::OnEndFrame.Remove(FrameHandle);
    Run();
    Record(TEXT("complete"), !Failed);
    FPlatformMisc::RequestExitWithStatus(false, Failed ? 1 : 0);
}
}

void RegisterConfirmationDescriptionProbeDriver()
{
    if (ConfirmationDescriptionProbe::Registered || !FParse::Param(FCommandLine::Get(), TEXT("ConfirmationDescriptionProbe"))) return;
    ConfirmationDescriptionProbe::Registered = true;
    ConfirmationDescriptionProbe::FrameHandle = FCoreDelegates::OnEndFrame.AddStatic(&ConfirmationDescriptionProbe::Tick);
}
