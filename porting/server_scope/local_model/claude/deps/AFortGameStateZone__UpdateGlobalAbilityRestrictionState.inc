// Native 0x141ea5f80. CONFIDENCE: MEDIUM. Reads the global ability-restriction tag (FFortGlobals name pair at +0x788/+0x78c). It is "active" when that tag is present in this->ActiveTags (+0x8a8, count +0x8b0) or this->ActiveParentTags (+0x8b8, count +0x8c0). The state is then applied to the sub-object at +0x710 when active, or +0x728 when not (helper 0x140684de0, not ported line by line; proposed name ApplyRestrictionSubobjectState).
bool bActive = false;
FGameplayTag GlobalTag = GetFortGlobals()->GlobalAbilityRestrictionTag;
if (!GlobalTag.IsNone())
{
	bActive = ActiveTags.Contains(GlobalTag) || ActiveParentTags.Contains(GlobalTag);
}
ApplyRestrictionSubobjectState(bActive ? RestrictionSubobjectActive : RestrictionSubobjectInactive);
