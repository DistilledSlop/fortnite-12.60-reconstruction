# World inventory multi-item lookup

Original `142006b20`, cached in `generated/native-cap-helpers30/`, iterates the real Inventory.ItemInstances, skips null items, dynamically compares GetItemDefinitionBP with the requested definition, and dynamically checks NativeWasGifted unless gifted items are allowed. It appends matching pointers in order and returns the count added. The requested definition can be null and the output array is not cleared. Existing single-item helper `1420065a0` has a different null-definition gate, which must not be copied here.

Root reviewed the complete cached body against `inventory-allitems214`'s proposal. The controller/profile all-items paths and installed backend remain incomplete; this helper alone does not establish complete inventory interface bindings. Build/runtime acceptance is pending.
