// CONFIDENCE: MEDIUM. Exec thunk 0x14292b8b0 copies TagsToBlock from its frame, then calls native 0x141ea02a0 (deps AFortGameStateZone__ApplyGlobalAbilityBlockTags). Behaviour: when AbilityRestrictionState (+0xf0) is 3, each tag gets +1 in the tag count container (+0x780, not explicit), the explicit tag container (+0x888) is copied to the active container (+0x8a8), and the restriction state is refreshed.
// Original address: 0x14292b8b0 (exec).
ApplyGlobalAbilityBlockTags(TagsToBlock);
