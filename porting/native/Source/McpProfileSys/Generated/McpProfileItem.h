// Generated from profile_entry_lifecycle_runtime; do not edit by hand.
#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonValue.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UMcpProfile;
class UObject;
class FReferenceCollector;

// Native backing item, distinct from the reflected profile-group FProfileEntry.
// These are semantic fields, not an assertion about the shipping binary layout.
struct MCPPROFILESYS_API FMcpProfileItem
{
    explicit FMcpProfileItem(const FString& InItemId);
    FMcpProfileItem(const FMcpProfileItem& Other);
    FMcpProfileItem& operator=(const FMcpProfileItem& Other);
    ~FMcpProfileItem();

    // The exact interpretation of the first lookup key is not yet recovered.
    FString AlternateDefinitionKey;
    FString ItemId;
    FString TemplateId;
    int32 Quantity;
    TMap<FString, TSharedPtr<FJsonValue>> Attributes;
    UObject* ItemObject;
    int32 NativeUpdateValue; // assigned name; semantic meaning not yet established
    TWeakObjectPtr<UMcpProfile> Profile;

    // The containing profile/backing must invoke this during reference collection.
    void AddReferencedObjects(FReferenceCollector& Collector);
};
