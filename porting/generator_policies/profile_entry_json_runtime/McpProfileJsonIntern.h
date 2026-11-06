#pragma once
#include "CoreMinimal.h"
#include "Dom/JsonValue.h"

struct FMcpProfileJsonInternPool
{
    TSharedPtr<FJsonValue> EmptyArray;
    TSharedPtr<FJsonValue> EmptyObject;
    TSharedPtr<FJsonValue> EmptyString;
    TSharedPtr<FJsonValue> Zero;
    TSharedPtr<FJsonValue> One;
    TSharedPtr<FJsonValue> True;
    TSharedPtr<FJsonValue> False;
};

MCPPROFILESYS_API const FMcpProfileJsonInternPool& NativeGetProfileJsonInternPool();
