// CONFIDENCE: high; original native calls the item definition virtual and checks its concrete class.
// Exec 0x1427ae690 calls virtual GetItemDefinitionBP and checks the concrete class.
return Cast<UFortAlterationItemDefinition>(GetItemDefinitionBP());
