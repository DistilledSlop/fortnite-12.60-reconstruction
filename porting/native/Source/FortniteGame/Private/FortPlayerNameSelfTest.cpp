// Opt-in dedicated-server integration test for the recovered player-name lifecycle.
#include "AFortPlayerState.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Serialization/BitReader.h"
#include "Serialization/BitWriter.h"

namespace
{
FDelegateHandle TickHandle;
bool bFinished = false;
int32 Failures = 0;

FString Units(const uint16* Data, int32 Count)
{
    static_assert(sizeof(TCHAR) == sizeof(uint16), "The original name codec uses UTF-16 units");
    FString Result;
    if (Count)
    {
        Result.GetCharArray().AddUninitialized(Count + 1);
        FMemory::Memcpy(Result.GetCharArray().GetData(), Data, Count * sizeof(uint16));
        Result.GetCharArray()[Count] = 0;
    }
    return Result;
}

template <size_t N> FString Units(const uint16 (&Data)[N]) { return Units(Data, N); }

void Check(const TCHAR* Id, bool bOk)
{
    if (!bOk) { ++Failures; UE_LOG(LogTemp, Error, TEXT("FortPlayerNameSelfTest FAIL %s"), Id); }
    else { UE_LOG(LogTemp, Display, TEXT("FortPlayerNameSelfTest PASS %s"), Id); }
}

bool Equal(const FString& A, const FString& B)
{
    return A.Len() == B.Len() && (A.Len() == 0 || FMemory::Memcmp(*A, *B, A.Len() * sizeof(TCHAR)) == 0);
}

void Archive(const TCHAR* Id, const FString& Source, const FString& Expected)
{
    FBitWriter Writer(8192, true);
    FString Written = Source;
    Writer << Written;
    FBitReader Reader(Writer.GetData(), Writer.GetNumBits());
    FString Loaded;
    Reader << Loaded;
    Check(Id, !Writer.IsError() && !Reader.IsError() && Reader.GetPosBits() == Writer.GetNumBits() && Equal(Loaded, Expected));
}

void Run(UWorld* World)
{
    // These constants are outputs of the original x86 loop oracle, not the implementation under test.
    const uint16 Local[] = {76,111,99,97,108,80,108,97,121,101,114};
    const uint16 Local0[] = {70,110,95,90,106,75,108,94,115,100,110};
    const uint16 Local1[] = {73,105,98,93,101,78,103,97,118,95,113};
    const uint16 Local2[] = {76,111,99,97,108,80,108,97,121,101,114,50};
    const uint16 Local20[] = {73,105,98,93,101,78,103,97,118,95,113,46};
    const uint16 Local21[] = {72,104,97,92,108,77,102,96,117,94,112,45};
    const uint16 Bmp[] = {0x4e2d,0x00e9};
    const uint16 Bmp0[] = {20008,233};
    const uint16 Bmp1[] = {20007,232};
    const uint16 Surrogate[] = {0xd83d,0xde00};
    const uint16 Surrogate0[] = {55352,56832};
    const uint16 Surrogate1[] = {55351,56831};
    const uint16 Shift[] = {4};
    const uint16 Shift0[] = {0};
    const uint16 Shift1[] = {65535};
    const uint16 Wrap[] = {1};
    const uint16 Wrap0[] = {65533};
    const uint16 Wrap1[] = {65532};
    struct FCase { const TCHAR* Id; FString Plain, Current, Old; };
    const FCase Cases[] = {
        {TEXT("empty"), FString(), FString(), FString()},
        {TEXT("local"), Units(Local), Units(Local0), Units(Local1)},
        {TEXT("local2"), Units(Local2), Units(Local20), Units(Local21)},
        {TEXT("bmp"), Units(Bmp), Units(Bmp0), Units(Bmp1)},
        {TEXT("surrogate"), Units(Surrogate), Units(Surrogate0), Units(Surrogate1)},
        {TEXT("shift_nul"), Units(Shift), Units(Shift0), Units(Shift1)},
        {TEXT("wrap"), Units(Wrap), Units(Wrap0), Units(Wrap1)}
    };
    FActorSpawnParameters Params;
    Params.ObjectFlags |= RF_Transient;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AFortPlayerState* State = World->SpawnActor<AFortPlayerState>(AFortPlayerState::StaticClass(), FTransform::Identity, Params);
    if (!State) { Check(TEXT("spawn"), false); return; }
    for (const FCase& C : Cases)
    {
        State->SetPlayerName(C.Plain);
        Check(C.Id, Equal(State->APlayerState::GetPlayerNameCustom(), C.Current)
            && Equal(State->GetPlayerName(), C.Plain)
            && Equal(State->APlayerState::GetOldPlayerName(), C.Old)
            && Equal(State->GetOldPlayerName(), C.Plain));
    }
    State->SetPlayerName(Units(Local));
    State->SetPlayerNameInternal(Units(Local2));
    Check(TEXT("internal_preserves_old"), Equal(State->APlayerState::GetPlayerNameCustom(), Units(Local20))
        && Equal(State->GetPlayerName(), Units(Local2)) && Equal(State->APlayerState::GetOldPlayerName(), Units(Local1))
        && Equal(State->GetOldPlayerName(), Units(Local)));
    State->SetOldPlayerName(Units(Local21));
    Check(TEXT("set_old_raw"), Equal(State->APlayerState::GetOldPlayerName(), Units(Local21)) && Equal(State->GetOldPlayerName(), Units(Local2)));
    State->SetIsOnlyASpectator(true);
    State->SetPlayerName(Units(Local));
    Check(TEXT("spectator"), Equal(State->APlayerState::GetPlayerNameCustom(), Units(Local))
        && Equal(State->GetPlayerName(), Units(Local)) && Equal(State->APlayerState::GetOldPlayerName(), Units(Local))
        && Equal(State->GetOldPlayerName(), Units(Local)));
    State->Destroy();

    Archive(TEXT("archive_empty"), FString(), FString());
    Archive(TEXT("archive_local"), Units(Local0), Units(Local0));
    Archive(TEXT("archive_local2"), Units(Local20), Units(Local20));
    Archive(TEXT("archive_bmp"), Units(Bmp0), Units(Bmp0));
    Archive(TEXT("archive_surrogate"), Units(Surrogate0), Units(Surrogate0));
    Archive(TEXT("archive_wrap"), Units(Wrap0), Units(Wrap0));
    Archive(TEXT("archive_shift_nul"), Units(Shift0), Units(Shift0));
    Archive(TEXT("archive_old_ffff"), Units(Shift1), FString());
    const uint16 FfffInput[] = {0x41, 0xffff, 0x42};
    const uint16 FfffPrefix[] = {0x41};
    Archive(TEXT("archive_ffff_truncates"), Units(FfffInput), Units(FfffPrefix));
    UE_LOG(LogTemp, Display, TEXT("FortPlayerNameSelfTest archive limitation: FString loading truncates at U+FFFF; arbitrary UTF-16 unit transport is not established"));
}

void OnWorldTick(UWorld* World, ELevelTick, float)
{
    if (bFinished || !World || !World->IsGameWorld() || World->GetNetMode() != NM_DedicatedServer) return;
    bFinished = true;
    FWorldDelegates::OnWorldTickStart.Remove(TickHandle);
    Run(World);
    UE_LOG(LogTemp, Display, TEXT("FortPlayerNameSelfTest complete failures=%d"), Failures);
    FPlatformMisc::RequestExitWithStatus(false, Failures ? 1 : 0);
}
}

void RegisterFortPlayerNameSelfTest()
{
    if (FParse::Param(FCommandLine::Get(), TEXT("FortnitePlayerNameSelfTest")))
        TickHandle = FWorldDelegates::OnWorldTickStart.AddStatic(&OnWorldTick);
}
