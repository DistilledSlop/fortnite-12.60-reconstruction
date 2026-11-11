// CONFIDENCE: MEDIUM Writes the current ability source level through the out parameter. The native calls the source-level core with the ability spec handle (+0x388) and actor info (+0x380).
// Original: exec 0x1428f8380 (decompile captures/0da226f5ed8442fb9ce91e4985f6c96a/0x1428f8380.c)
OutSourceLevel = GetAbilitySourceLevelForSpec(CurrentSpecHandle, CurrentActorInfo);
