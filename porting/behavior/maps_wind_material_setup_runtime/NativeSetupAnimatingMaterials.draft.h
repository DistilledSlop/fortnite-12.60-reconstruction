// Complete proposed definition; not integrated or build verified.
// The .inc is authoritative for root promotion; see evidence.json.
#pragma once
#include "AFortWindManager.h"
#include "ABuildingSMActor.h"
#include "FFortWindResponder.h"
#include "Curves/CurveLinearColor.h"
#include "Components/MeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "Kismet/KismetMaterialLibrary.h"

#include "SharedFortGameLog.h"

inline void AFortWindManager::NativeSetupAnimatingMaterials(FFortWindResponder& Responder)
{
#include "AFortWindManager_NativeSetupAnimatingMaterials.inc"
}
