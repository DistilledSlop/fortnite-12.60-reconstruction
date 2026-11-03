#include "McpProfileItem.h"

#include "UMcpProfile.h"
#include "UObject/GCObject.h"

FMcpProfileItem::FMcpProfileItem(const FString& InItemId)
    : ItemId(InItemId), Quantity(1), ItemObject(nullptr), NativeUpdateValue(INDEX_NONE)
{
}

FMcpProfileItem::FMcpProfileItem(const FMcpProfileItem& Other) = default;
FMcpProfileItem& FMcpProfileItem::operator=(const FMcpProfileItem& Other) = default;
FMcpProfileItem::~FMcpProfileItem() = default;

void FMcpProfileItem::AddReferencedObjects(FReferenceCollector& Collector)
{
    Collector.AddReferencedObject(ItemObject);
}
