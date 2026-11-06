// CONFIDENCE: HIGH
// Item must be valid. Each index retains its own shared reference.
ItemsById.Add(Item->ItemId, Item);
NativeInsertProfileGroup(Groups, Item->AlternateDefinitionKey, Item);
NativeInsertProfileGroup(Groups, Item->TemplateId, Item);
