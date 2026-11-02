#pragma once

// Unregistered source candidate for node+98 elements, not a replacement router.
// Project-local descriptive identifier; original private identifier is unknown.
#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"

struct FCommonActionRouteInputPreprocessorEntry
{
    int32 Index;
    TSharedPtr<IInputProcessor> InputProcessor;
};

static_assert(sizeof(FCommonActionRouteInputPreprocessorEntry) == 0x18,
              "Original node+98 entries use a 0x18 stride");
static_assert(STRUCT_OFFSET(FCommonActionRouteInputPreprocessorEntry, Index) == 0,
              "Original processor index is at entry+0");
static_assert(STRUCT_OFFSET(FCommonActionRouteInputPreprocessorEntry, InputProcessor) == 8,
              "Original shared input processor starts at entry+8");
