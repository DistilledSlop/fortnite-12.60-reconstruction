// CONFIDENCE: HIGH. Complete original body; class lifecycle acceptance remains separate.
//AFortAthenaMutator_PlayerSettingsBase override141afe3e0 (inherited Class/Team/Zone).
const FName Name(TEXT("bApplyJumpPenalty"));
if (UFortPlayerSettingsComponentBase* Settings = NativeFindEnabledPlayerSettings163(Name, true))
{
    bApply = Settings->bApplyJumpPenalty; // reflected+198
    return 2;
}
return 0; // Original null RAX low byte is returned, does not change bApply.
