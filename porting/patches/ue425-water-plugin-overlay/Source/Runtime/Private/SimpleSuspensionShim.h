// Plugin-local shim for Engine/Public/Physics/SimpleSuspension.h, which exists only in UE 4.26.
// The UE 4.25 engine checkout has no such header, and this round may not patch engine source.
// Only FSimpleSuspensionHelpers::ComputeSprungMasses is used (BuoyancyComponent pontoon coefficients).
// One and two spring cases follow the 4.26 formulas. Three or more springs use an equal split,
// which is an approximation: it keeps the total mass but does not solve the 4.26 centre-of-mass constraint.
#pragma once

#include "CoreMinimal.h"
#include "Containers/Array.h"
#include "Math/UnrealMathUtility.h"

struct FSimpleSuspensionHelpers
{
	static bool ComputeSprungMasses(const TArray<FVector>& LocalSpringPositions, const FVector& LocalCenterOfMass, const float TotalMass, TArray<float>& OutSprungMasses)
	{
		const int32 Count = LocalSpringPositions.Num();
		OutSprungMasses.SetNumZeroed(Count);
		if (Count <= 0 || TotalMass <= SMALL_NUMBER)
		{
			return false;
		}

		if (Count == 1)
		{
			OutSprungMasses[0] = TotalMass;
			return true;
		}

		if (Count == 2)
		{
			const float AX = LocalSpringPositions[0].X - LocalCenterOfMass.X;
			const float AY = LocalSpringPositions[0].Y - LocalCenterOfMass.Y;
			const float DiffX = (LocalSpringPositions[1].X - LocalCenterOfMass.X) - AX;
			const float DiffY = (LocalSpringPositions[1].Y - LocalCenterOfMass.Y) - AY;
			const float Dist = FMath::Sqrt((DiffX * DiffX) + (DiffY * DiffY));
			if (Dist <= SMALL_NUMBER)
			{
				OutSprungMasses[0] = OutSprungMasses[1] = TotalMass * 0.5f;
				return true;
			}
			const float DistInv = 1.f / Dist;
			const float DirX = DiffX * DistInv;
			const float DirY = DiffY * DistInv;
			const float DirDotA = (DirX * AX) + (DirY * AY);
			OutSprungMasses[0] = -TotalMass * DirDotA * DistInv;
			OutSprungMasses[1] = TotalMass - OutSprungMasses[0];
			return OutSprungMasses[0] >= 0.f && OutSprungMasses[1] >= 0.f;
		}

		// Approximation for N >= 3 (see header comment).
		for (int32 Index = 0; Index < Count; ++Index)
		{
			OutSprungMasses[Index] = TotalMass / Count;
		}
		return true;
	}
};
