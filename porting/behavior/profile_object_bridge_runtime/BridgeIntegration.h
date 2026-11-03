#pragma once

// Integration fragments: add these members to the indicated existing classes.
// This file is documentation as C++, not a standalone header.
#if 0
// IMcpItemAware, after GENERATED_IINTERFACE_BODY():
public:
    virtual bool PopulateFromMcpItem(FMcpProfileItem* Item) = 0;
    TWeakPtr<FMcpProfileItem, ESPMode::NotThreadSafe> McpItem;

// UMcpProfile, in its non-reflected members:
public:
    // Assigned names: no original C++ identifiers are claimed.
    TMap<FString, UClass*> ItemClassRegistry;
    virtual UObject* CreateItemObject(UClass* ItemClass, FString ItemId, FString TemplateId);
    void UpdateItemObject(TSharedPtr<FMcpProfileItem, ESPMode::NotThreadSafe> Item, bool bForcePopulate);
#endif
