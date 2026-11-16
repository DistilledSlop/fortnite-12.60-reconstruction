// Diagnostic: finds primitives that should block on the server but have no physics body there.
//
// The 2026-11-15 server log has 514 "ConvexElem is missing ConvexMesh", 257 "ScaledElem invalid" and 257 "convex cooking failed" warnings for
// terrain spline meshes. A component with collision enabled and no valid body instance is invisible to server traces and to the server's
// character movement, which shows up as walking through scenery or a position correction on the client that does see the shape.
// Run: console command `FortCollisionAudit` (server), or start with -FortniteCollisionAudit (runs once, 90 s after the first call from
// FortCollisionAudit::Tick). Output: `CollisionAudit:` lines, one per level with the count and up to three examples.
#include "FortCollisionAudit.h"
#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodySetup.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortCollisionAudit, Log, All);

namespace
{
	struct FLevelStats
	{
		int32 Checked = 0;
		int32 NoBody = 0;
		TArray<FString> Examples;
	};

	void RunAudit(UWorld* World)
	{
		if (!World)
		{
			return;
		}
		TMap<FString, FLevelStats> PerLevel;
		int32 Total = 0, Missing = 0, HighCount = 0;
		constexpr float HighAltitude = 20000.0f;
		TArray<FString> HighExamples;
		for (TObjectIterator<UPrimitiveComponent> It; It; ++It)
		{
			UPrimitiveComponent* Comp = *It;
			if (!Comp || Comp->GetWorld() != World || Comp->IsTemplate() || !Comp->IsRegistered())
			{
				continue;
			}
			if (Comp->GetCollisionEnabled() == ECollisionEnabled::NoCollision || Comp->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Ignore)
			{
				continue;
			}
			// Glider diagnostic: the client's parachute auto-deploy traces down at most ~10000 units, and the server log showed the glider
			// opening ~49000 units up. Anything solid that high would be hit by that trace, so list every blocking component of any type
			// whose top is above 20000 units.
			{
				const FBoxSphereBounds Bounds = Comp->Bounds;
				const float Top = Bounds.Origin.Z + Bounds.BoxExtent.Z;
				if (Top > HighAltitude)
				{
					++HighCount;
					if (HighExamples.Num() < 25)
					{
						HighExamples.Add(FString::Printf(TEXT("%s class=%s profile=%s z=[%.0f..%.0f] extent=(%.0f,%.0f)"), *Comp->GetPathName().Right(110),
							*Comp->GetClass()->GetName(), *Comp->GetCollisionProfileName().ToString(), Bounds.Origin.Z - Bounds.BoxExtent.Z, Top,
							Bounds.BoxExtent.X, Bounds.BoxExtent.Y));
					}
				}
			}
			if (!Comp->IsA<UStaticMeshComponent>())
			{
				continue; // scenery only; skeletal meshes and shapes are handled by their owners
			}
			++Total;
			FLevelStats& Stats = PerLevel.FindOrAdd(Comp->GetComponentLevel() ? Comp->GetComponentLevel()->GetOutermost()->GetName() : FString(TEXT("unknown")));
			++Stats.Checked;
			const FBodyInstance* Body = Comp->GetBodyInstance();
			if (!Body || !Body->IsValidBodyInstance())
			{
				++Missing;
				++Stats.NoBody;
				if (Stats.Examples.Num() < 3)
				{
					const FVector Loc = Comp->GetComponentLocation();
					Stats.Examples.Add(FString::Printf(TEXT("%s@(%.0f,%.0f,%.0f)"), *Comp->GetPathName().Right(90), Loc.X, Loc.Y, Loc.Z));
				}
			}
		}
		UE_LOG(LogFortCollisionAudit, Display, TEXT("CollisionAudit: %d pawn-blocking static mesh components, %d without a physics body, %d levels"), Total, Missing, PerLevel.Num());
		UE_LOG(LogFortCollisionAudit, Display, TEXT("CollisionAudit: %d blocking components reach above %.0f units (glider deploy trace is ~10000)"), HighCount, HighAltitude);
		for (const FString& Example : HighExamples)
		{
			UE_LOG(LogFortCollisionAudit, Display, TEXT("CollisionAudit: high %s"), *Example);
		}
		PerLevel.ValueSort([](const FLevelStats& A, const FLevelStats& B) { return A.NoBody > B.NoBody; });
		int32 Printed = 0;
		for (const TPair<FString, FLevelStats>& Pair : PerLevel)
		{
			if (Pair.Value.NoBody == 0 || Printed++ >= 40)
			{
				break;
			}
			UE_LOG(LogFortCollisionAudit, Display, TEXT("CollisionAudit: level=%s noBody=%d of %d examples=%s"), *Pair.Key, Pair.Value.NoBody, Pair.Value.Checked, *FString::Join(Pair.Value.Examples, TEXT(" | ")));
		}
	}

	FAutoConsoleCommandWithWorld GAuditCommand(TEXT("FortCollisionAudit"), TEXT("Logs scenery static meshes that block pawns but have no physics body on this server."),
		FConsoleCommandWithWorldDelegate::CreateStatic(&RunAudit));
}

void FortCollisionAudit::Tick(UWorld* World)
{
	static double FirstCall = -1.0;
	static bool bDone = false;
	if (bDone || !World || !FParse::Param(FCommandLine::Get(), TEXT("FortniteCollisionAudit")))
	{
		return;
	}
	if (FirstCall < 0.0)
	{
		FirstCall = World->GetTimeSeconds();
	}
	if (World->GetTimeSeconds() - FirstCall >= 90.0)
	{
		bDone = true;
		RunAudit(World);
	}
}
