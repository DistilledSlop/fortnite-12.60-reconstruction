// Plugin-local stub replacing the UE 4.26 DynamicMesh boolean implementation of this file.
// The 4.25 GeometryProcessing checkout has no Operations/MeshBoolean.h. Only BuildOceanCollisionComponents is used
// (WaterBodyOceanActor.cpp). This stub emits no ocean collision boxes or convex sets, so the ocean has no generated
// collision in this port. Exclusion volumes are not applied.
#pragma once

#include "CoreMinimal.h"
#include "Math/Box.h"
#include "Engine/EngineTypes.h"
#include "Math/BoxSphereBounds.h"
#include "PhysicsEngine/ConvexElem.h"
#include "WaterBodyExclusionVolume.h"
#include "WaterModule.h"

namespace UE
{
	namespace WaterUtils
	{
		inline void BuildOceanCollisionComponents(
			FBoxSphereBounds WorldBoxIn,
			FTransform ActorTransform,
			const TArray<AWaterBodyExclusionVolume*>& ExclusionVolumes,
			TArray<FBoxSphereBounds>& Boxes,
			TArray<TArray<FKConvexElem>>& MeshConvexes,
			double WorldMeshBufferWidth = 1000.0,
			double WorldBoxOverlap = 10.0)
		{
			Boxes.Reset();
			MeshConvexes.Reset();
		}
	}
}
