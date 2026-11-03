// CONFIDENCE: high; complete native storage query and reflected fields.
// The native body returns the manager from the first pair, if one exists.
return DataManagers.Num() > 0 ? DataManagers[0].EncounterDataManager : nullptr;
