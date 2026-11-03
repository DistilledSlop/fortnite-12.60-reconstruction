// Reconstruction of 0x142025400. Pending the concrete native member of
// FFortGiftingInfo; see HANDOFF.md. This is not a manifest-ready fragment.
// Destination and Source are existing FFortItemEntry objects.
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
// Native 0x142025370: copies PlayerName, HeroType and the +0x18 shared
// member, retaining the incoming reference before releasing the old one.
// Destination.GiftingInfo = Source.GiftingInfo; // only after native member is restored
Destination.bWasGifted = Source.bWasGifted;
Destination.StateValues = Source.StateValues;
Destination.GameplayAbilitySpecHandle = Source.GameplayAbilitySpecHandle;
Destination.GenericAttributeValues = Source.GenericAttributeValues;
Destination.bUpdateStatsOnCollection = Source.bUpdateStatsOnCollection;
Destination.WrapOverride = Source.WrapOverride;
// Fast-array base, ParentInventory, bIsReplicatedCopy, and bIsDirty untouched.
