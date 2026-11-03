// Integration fragment, not a standalone generated header.
// Add to IMcpItemAware after GENERATED_IINTERFACE_BODY().
// FMcpProfileItem is the logical type in profile_entry_lifecycle_runtime.
protected:
    TWeakPtr<FMcpProfileItem, ESPMode::NotThreadSafe> McpItem;
public:
    // The base declaration's original pure/default status remains unresolved.
    virtual bool PopulateFromMcpItem(FMcpProfileItem* Item) = 0;

// Add IMcpItemAware as a native interface base of UFortItem, preserving UHT
// interface registration. Add the following to UFortItem's declaration:
// virtual bool PopulateFromMcpItem(FMcpProfileItem* Item) override;
