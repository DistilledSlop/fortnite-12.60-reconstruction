#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"
#include "JsonObjectConverter.h"

// Proposed body for 0x1410f59b0; the caller supplies the native reflected type.
// JsonUtilities must be linked. The caller owns OutStruct and its lifetime.
inline bool DeserializeMcpAttributes(
    const TMap<FString, TSharedPtr<FJsonValue>>& Attributes,
    void* OutStruct,
    const UStruct* StructDefinition)
{
    TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
    for (const auto& Attribute : Attributes)
    {
        Object->Values.Add(Attribute.Key, Attribute.Value);
    }
    return FJsonObjectConverter::JsonAttributesToUStruct(
        Object->Values, StructDefinition, OutStruct, 0, 0);
}
