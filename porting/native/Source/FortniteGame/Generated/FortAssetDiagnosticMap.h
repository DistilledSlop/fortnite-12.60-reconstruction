#pragma once

#include "Containers/Map.h"
#include "UObject/SoftObjectPath.h"

// Shared type for the actual diagnostic maps supplied by their eventual owner.
// The default key policy is provisional until the original mutation policy is recovered.
using FFortAssetDiagnosticMap = TMap<FSoftObjectPath, int32>;
