// CONFIDENCE: high
// Native target 0x1424e40c0 reads ZoneTheme at 0xb78 and SupportedUseType at 0x209.
if (!ZoneTheme)
{
    return EDeployableBaseUseType::Neighborhood;
}
UFortZoneTheme* CurrentTheme = ZoneTheme;
return CurrentTheme->DeployableBaseSettings.SupportedUseType;
