#include "FortAthenaMiscRpcs.h"
#include "FortAthenaHealth.h"
#include "AFortPlayerPawn.h"
#include "FZiplinePawnState.h"
#include "AFortAthenaZipline.h"
#include "UObject/ScriptMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortMiscRpc, Log, All);

namespace
{
	// ServerSendZiplineState: the client owns the zipline motion (client-authoritative custom movement mode 8) and tells the
	// server its state so it replicates to the other clients (AFortPlayerPawn::ZiplineState is replicated).
	// binary: original body 0x1406801c0 is a bare RET (stripped).
	// stand-in: AuthoritativeValue (client counter, FZiplinePawnState +0x?? "AuthoritativeValue") is used as a monotonic
	//   sequence number: an older state never overwrites a newer one. Original conflict rule not recovered.
	void ExecServerSendZiplineState(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_STRUCT_REF(FZiplinePawnState, InZiplineState);
		P_FINISH;
		P_NATIVE_BEGIN;
		if (AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(Context))
		{
			const bool bNewer = InZiplineState.AuthoritativeValue >= Pawn->ZiplineState.AuthoritativeValue;
			UE_LOG(LogFortMiscRpc, Display, TEXT("Zipline: %s state ziplining=%d jumped=%d zipline=%s value=%d (current %d) -> %s"), *Pawn->GetName(),
				InZiplineState.bIsZiplining ? 1 : 0, InZiplineState.bJumped ? 1 : 0, *GetNameSafe(InZiplineState.Zipline),
				InZiplineState.AuthoritativeValue, Pawn->ZiplineState.AuthoritativeValue, bNewer ? TEXT("accepted") : TEXT("stale, ignored"));
			if (bNewer)
			{
				Pawn->ZiplineState = InZiplineState;
				Pawn->ForceNetUpdate();
			}
		}
		P_NATIVE_END;
	}
}

void FortAthenaMiscRpcs::InstallRpcOverrides()
{
	static bool bInstalled = false;
	if (bInstalled || !FortAthenaHealth::IsEnabled())
		return;
	bInstalled = true;
	if (UFunction* Function = AFortPlayerPawn::StaticClass()->FindFunctionByName(TEXT("ServerSendZiplineState")))
	{
		Function->SetNativeFunc(&ExecServerSendZiplineState);
		UE_LOG(LogFortMiscRpc, Display, TEXT("Zipline: AFortPlayerPawn::ServerSendZiplineState rebound to the re-created body"));
	}
	else
	{
		UE_LOG(LogFortMiscRpc, Warning, TEXT("Zipline: ServerSendZiplineState UFunction not found"));
	}
}
