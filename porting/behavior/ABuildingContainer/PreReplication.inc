// Original address: 0x0 (the retail ABuildingContainer override is unknown).
// CONFIDENCE: low
// stand-in: bAlreadySearched is replicated with COND_Custom (replication_conditions.json), and without an override that turns the
// condition on the property is never sent, so clients never saw a searched chest (no open animation or sound, prompt stayed).
// Battle Royale containers are not instanced per player, so the flag goes to every connection.
Super::PreReplication(ChangedPropertyTracker);
DOREPLIFETIME_ACTIVE_OVERRIDE(ABuildingContainer, bAlreadySearched, true);
