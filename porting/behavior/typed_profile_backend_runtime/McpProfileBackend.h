#pragma once

#include "CoreMinimal.h"
#include "McpProfileItem.h"

// Logical support class: no claim of shipping layout or complete owner behavior.
class MCPPROFILESYS_API FMcpProfileBackend
{
public:
    using FItemPtr = TSharedPtr<FMcpProfileItem, ESPMode::NotThreadSafe>;
    using FItemArray = TArray<FItemPtr>;
    using FItemMap = TMap<FString, FItemPtr>;
    using FGroupMap = TMap<FString, FItemArray>;

    FMcpProfileBackend();
    virtual ~FMcpProfileBackend();

    // Returned array belongs to this backend and may be invalidated by mutation.
    virtual const FItemArray* FindGroup(const FString& Key) const;
    const FItemArray* FindBaseGroup(const FString& Key) const;
    void InsertItem(const FItemPtr& Item);

private:
    static void InsertGroup(FGroupMap& Groups, const FString& Key, const FItemPtr& Item);

    FItemMap ItemsById;
    FGroupMap Groups;
};
