// CONFIDENCE: MEDIUM. Original exec 0x145262c20 (self-contained; helpers 0x142ebd0a0, 0x142ee3b30, 0x141f7ce30, 0x142ebd9e0).
// Pairs an item primary asset id with a quantity. A null definition produces an invalid id with the requested quantity.
FFortItemQuantityPair Pair;
Pair.ItemPrimaryAssetId = IsValid(ItemDefinition) ? ItemDefinition->GetPrimaryAssetId() : FPrimaryAssetId();
Pair.Quantity = ItemQuantity;
return Pair;
