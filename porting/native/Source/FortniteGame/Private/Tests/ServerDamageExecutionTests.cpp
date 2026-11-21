#if WITH_DEV_AUTOMATION_TESTS

#include "AFortPawn.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "FortAthenaHealth.h"
#include "GameFramework/DamageType.h"
#include "Misc/AutomationTest.h"
#include "ServerDamageExecution.h"
#include "UFortAbilitySystemComponent.h"
#include "UFortHealthSet.h"

// UE 4.25 discovers dedicated-server tests under ClientContext.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FServerDamageExecutionTest, "Fortnite.Server.Damage.UnrealEvent",
	EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext | EAutomationTestFlags::EngineFilter)

bool FServerDamageExecutionTest::RunTest(const FString&)
{
	if (!IsRunningDedicatedServer() || !FortAthenaHealth::IsEnabled())
	{
		AddInfo(TEXT("Requires a dedicated server launched with -FortniteOriginalClient."));
		return true;
	}

	UWorld* World = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.World() && Context.World()->GetNetMode() == NM_DedicatedServer)
		{
			World = Context.World();
			break;
		}
	}
	if (!TestNotNull(TEXT("Dedicated server world"), World))
	{
		return false;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AFortPawn* Pawn = World->SpawnActor<AFortPawn>(FVector(0, 0, 10000), FRotator::ZeroRotator, SpawnParameters);
	if (!TestNotNull(TEXT("Damage test pawn"), Pawn))
	{
		return false;
	}

	UFortAbilitySystemComponent* ASC = NewObject<UFortAbilitySystemComponent>(Pawn);
	Pawn->AbilitySystemComponent = ASC;
	ASC->RegisterComponent();
	ASC->InitAbilityActorInfo(Pawn, Pawn);
	Pawn->HealthSet = NewObject<UFortHealthSet>(Pawn);
	ASC->AddAttributeSetSubobject(Pawn->HealthSet);
	Pawn->bHealthSynced = true;

	const FGameplayAttribute Health(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("Health")));
	const FGameplayAttribute Shield(FindFProperty<FProperty>(UFortHealthSet::StaticClass(), TEXT("CurrentShield")));
	ASC->SetNumericAttributeBase(Health, 100);
	ASC->SetNumericAttributeBase(Shield, 30);

	UServerPawnDamageListener::Bind(*Pawn);
	UServerPawnDamageListener::Bind(*Pawn);
	Pawn->TakeDamage(15, FDamageEvent(), nullptr, nullptr);
	TestEqual(TEXT("A damage event fires once and shield absorbs the hit"), Pawn->GetShield(), 15.0f);
	TestEqual(TEXT("Health is intact"), Pawn->GetHealth(), 100.0f);

	Pawn->TakeDamage(40, FDamageEvent(), nullptr, nullptr);
	TestEqual(TEXT("Shield is depleted"), Pawn->GetShield(), 0.0f);
	TestEqual(TEXT("Overflow reduces health"), Pawn->GetHealth(), 75.0f);

	ASC->SetNumericAttributeBase(Shield, 30);
	FortAthenaHealth::ApplyDamage(Pawn, 5, FGameplayTagContainer(), nullptr, nullptr, true);
	TestEqual(TEXT("Bypass preserves shield"), Pawn->GetShield(), 30.0f);
	TestEqual(TEXT("Bypass reduces health"), Pawn->GetHealth(), 70.0f);

	Pawn->TakeDamage(150, FDamageEvent(), nullptr, nullptr);
	TestEqual(TEXT("Lethal damage clamps health to zero"), Pawn->GetHealth(), 0.0f);
	TestEqual(TEXT("Lethal damage depletes the remaining shield"), Pawn->GetShield(), 0.0f);
	TestTrue(TEXT("A non-player pawn enters the dying state"), Pawn->bIsDying != 0);
	TestFalse(TEXT("A dying pawn stops accepting damage events"), Pawn->CanBeDamaged());

	Pawn->Destroy();
	return true;
}

#endif
