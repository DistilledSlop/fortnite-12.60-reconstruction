// Generated from profile_entry_lifecycle_runtime; do not edit by hand.
#pragma once

#include "CoreMinimal.h"
#include "McpProfileItem.h"

// Logical source types for the recovered indexes, not the native backing layout.
using FMcpProfileItemPtr = TSharedPtr<FMcpProfileItem, ESPMode::NotThreadSafe>;
using FMcpProfileItemArray = TArray<FMcpProfileItemPtr>;
using FMcpProfileItemMap = TMap<FString, FMcpProfileItemPtr>;
using FMcpProfileGroupMap = TMap<FString, FMcpProfileItemArray>;
