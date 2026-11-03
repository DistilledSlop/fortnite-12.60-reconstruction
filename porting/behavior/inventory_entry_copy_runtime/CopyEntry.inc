// CONFIDENCE: HIGH
Destination.Count = Source.Count;
Destination.PreviousCount = Source.PreviousCount;
Destination.ItemDefinition = Source.ItemDefinition;
Destination.OrderIndex = Source.OrderIndex;
Destination.Durability = Source.Durability;
Destination.Level = Source.Level;
Destination.LoadedAmmo = Source.LoadedAmmo;
Destination.AlterationDefinitions = Source.AlterationDefinitions;
Destination.AlterationInstances = Source.AlterationInstances;
Destination.ItemSource = Source.ItemSource;
Destination.ItemGuid = Source.ItemGuid;
Destination.ControlOverride = Source.ControlOverride;
Destination.inventory_overflow_date = Source.inventory_overflow_date;
NativeCopyGiftingInfo(Destination.GiftingInfo, Source.GiftingInfo);
Destination.bWasGifted = Source.bWasGifted;
Destination.StateValues = Source.StateValues;
Destination.GameplayAbilitySpecHandle = Source.GameplayAbilitySpecHandle;
Destination.GenericAttributeValues = Source.GenericAttributeValues;
Destination.bUpdateStatsOnCollection = Source.bUpdateStatsOnCollection;
Destination.WrapOverride = Source.WrapOverride;
// Fast-array base, ParentInventory, bIsReplicatedCopy, and bIsDirty untouched.
return Destination;
