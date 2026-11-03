// Draft constructor body. Requires the correctly typed non-reflected tail at
// +0x100..+0x14f, initialized by its own real constructor(s). See HANDOFF.md.
// FFastArraySerializerItem default constructor initializes all three keys to -1.
Count = 1;
PreviousCount = 0;
ItemDefinition = nullptr;
OrderIndex = -1;
Durability = 1.0f;
Level = 0;
LoadedAmmo = 0;
// AlterationDefinitions and ItemSource default construct empty; ItemGuid zeroes.
ControlOverride = 0;
inventory_overflow_date = false;
bWasGifted = false;
bIsReplicatedCopy = false;
bIsDirty = false;
bUpdateStatsOnCollection = true;
// GiftingInfo, StateValues, ParentInventory, GameplayAbilitySpecHandle,
// AlterationInstances, WrapOverride, and GenericAttributeValues default construct.
// ParentInventory is invalid (index -1, serial 0); ability handle is -1.
