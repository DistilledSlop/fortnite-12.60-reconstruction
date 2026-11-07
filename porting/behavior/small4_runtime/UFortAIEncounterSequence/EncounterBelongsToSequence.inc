// Original address: 0x141692a90
// Evidence: RDX = InEncounter compared with [this+0x60] (reflected CurrentEncounter, UFortAIEncounterInfo*). SETZ = pointer equality.
// CONFIDENCE: HIGH
return InEncounter == CurrentEncounter;
