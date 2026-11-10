// CONFIDENCE: MEDIUM. Exec thunk 0x14292b980 copies TagsToUnblock from its frame, then calls native 0x141ea0350 (deps AFortGameStateZone__ApplyGlobalAbilityUnblockTags). Behaviour: when AbilityRestrictionState (+0xf0) is 3, each tag gets -1 in the tag count container (+0x780); if any count changed, the explicit tag container (+0x888) is filled with parent tags; then the explicit container is copied to the active container (+0x8a8) and the restriction state is refreshed.
// Original address: 0x14292b980 (exec).
ApplyGlobalAbilityUnblockTags(TagsToUnblock);
