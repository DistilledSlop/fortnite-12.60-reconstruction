// CONFIDENCE: HIGH. Complete original body; all164 native mutator tables inventoried.
// 0x141afae00, inherited by class/team/zone settings.
const FName Name(TEXT("bGliderRedeploy"));
if (UFortPlayerSettingsComponentBase* Settings = NativeFindEnabledPlayerSettings163(Name, true))
{
    bAllowed = Settings->bGliderRedeploy;
    return 2;
}
return 0;
