// CONFIDENCE: MEDIUM. Exec thunk 0x14292a6b0 takes no parameters. It reads the global ability-restriction tag from FFortGlobals (+0x788, via GetFortGlobals), copies it into a container, and, when this->AbilityRestrictionState (+0xf0) equals 3, adds each tag to the tag count container (+0x780) with count +1 and no explicit flag, copies the explicit tag container (+0x888) into the active container (+0x8a8), and refreshes the restriction state (0x141ea5f80).
// Original address: 0x14292a6b0 (exec). Helpers: GetFortGlobals (deps AFortAIPawn__GetFortGlobals), UpdateTagCount (deps AFortGameplayTagCountContainer__UpdateTagCount), UpdateGlobalAbilityRestrictionState (deps AFortGameStateZone__UpdateGlobalAbilityRestrictionState).
FGameplayTagContainer GlobalTags = FGameplayTagContainer::FromTag(GetFortGlobals()->GlobalAbilityRestrictionTag);
FGameplayTagContainer RestrictionTags = GlobalTags;
if (AbilityRestrictionState == 3)
{
	for (const FGameplayTag& Tag : RestrictionTags.GameplayTags)
	{
		UpdateTagCount(GameplayTagCounts, Tag, 1, true);
	}
	ActiveTags = ExplicitTags;
	UpdateGlobalAbilityRestrictionState();
}
