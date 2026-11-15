#include "FortAthenaMiscRpcs.h"
#include "FortAthenaHealth.h"
#include "AFortPlayerPawn.h"
#include "AFortPlayerState.h"
#include "AFortPlayerController.h"
#include "FFortAthenaLoadout.h"
#include "FMcpVariantChannelInfo.h"
#include "GameplayTagContainer.h"
#include "FZiplinePawnState.h"
#include "AFortAthenaZipline.h"
#include "UObject/ScriptMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortMiscRpc, Log, All);

namespace
{
	// AFortPlayerState::ServerChangeActiveVariantForCosmeticItem (original body 0x1406801c0, bare RET: stripped).
	// stand-in: only the equipped character's variant channels (FFortAthenaLoadout.CharacterVariantChannels) exist on the
	//   server, so only those can change. The desired tag must be one the backend marked as owned for that channel
	//   (OwnedVariantTags); anything else is rejected. Items other than the equipped character are logged and ignored.
	//   Re-applying the character parts after the change is not done; the replicated loadout carries the new tag.
	bool ChangeChannel(TArray<FMcpVariantChannelInfo>& Channels, const FGameplayTag& Channel, const FGameplayTag& Desired)
	{
		for (FMcpVariantChannelInfo& Info : Channels)
		{
			if (Info.VariantChannelTag == Channel)
			{
				if (!Info.OwnedVariantTags.HasTagExact(Desired))
					return false;
				Info.ActiveVariantTag = Desired;
				return true;
			}
		}
		return false;
	}

	void ExecServerChangeActiveVariantForCosmeticItem(UObject* Context, FFrame& Stack, RESULT_DECL)
	{
		P_GET_PROPERTY_REF(FNameProperty, ItemTemplateToChange);
		P_GET_STRUCT_REF(FGameplayTag, VariantChannelToChange);
		P_GET_STRUCT_REF(FGameplayTag, DesiredActiveVariant);
		P_GET_OBJECT(AFortPlayerPawn, PreviewPawn);
		P_FINISH;
		P_NATIVE_BEGIN;
		AFortPlayerState* State = Cast<AFortPlayerState>(Context);
		AFortPlayerController* PC = State ? Cast<AFortPlayerController>(State->GetOwner()) : nullptr;
		AFortPlayerPawn* Pawn = PC ? Cast<AFortPlayerPawn>(PC->GetPawn()) : nullptr;
		bool bApplied = false;
		if (PC)
		{
			bApplied = ChangeChannel(PC->CosmeticLoadoutPC.CharacterVariantChannels, VariantChannelToChange, DesiredActiveVariant);
			if (Pawn && ChangeChannel(Pawn->CosmeticLoadout.CharacterVariantChannels, VariantChannelToChange, DesiredActiveVariant))
			{
				bApplied = true;
				Pawn->ForceNetUpdate();
			}
		}
		UE_LOG(LogFortMiscRpc, Display, TEXT("Variant: %s item=%s channel=%s desired=%s preview=%s -> %s"), *GetNameSafe(State), *ItemTemplateToChange.ToString(),
			*VariantChannelToChange.ToString(), *DesiredActiveVariant.ToString(), *GetNameSafe(PreviewPawn), bApplied ? TEXT("applied") : TEXT("rejected or not the equipped character"));
		P_NATIVE_END;
	}

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
	if (UFunction* VariantFunction = AFortPlayerState::StaticClass()->FindFunctionByName(TEXT("ServerChangeActiveVariantForCosmeticItem")))
	{
		VariantFunction->SetNativeFunc(&ExecServerChangeActiveVariantForCosmeticItem);
		UE_LOG(LogFortMiscRpc, Display, TEXT("Variant: AFortPlayerState::ServerChangeActiveVariantForCosmeticItem rebound to the re-created body"));
	}
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
