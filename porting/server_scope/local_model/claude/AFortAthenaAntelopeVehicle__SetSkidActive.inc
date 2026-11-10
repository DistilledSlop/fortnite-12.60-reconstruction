// CONFIDENCE: MEDIUM. Original exec 0x1427cfe80; native body 0x1418d3920 (inlined). Parameter: bActive (bool).
// Skid trails only run at visual quality level 2 or above. The effective state is compared with the cached state
// (flags bit 0 at +0x1158). On a change, both trail particle systems are started or stopped.
const bool bWantActive = bActive && (GetVisualQualityLevel() < 2);
if (bWantActive != ((NativeRecoveredByte_0x1158 & 1) != 0))
{
    if (!bWantActive)
    {
        if (PSC_TrailLeft)
        {
            DeactivateTrailEmitters(PSC_TrailLeft);
            PSC_TrailLeft->DeactivateSystem();
        }
        if (PSC_TrailRight)
        {
            DeactivateTrailEmitters(PSC_TrailRight);
            PSC_TrailRight->DeactivateSystem();
        }
    }
    else
    {
        if (PSC_TrailLeft)
        {
            PSC_TrailLeft->ActivateSystem(false);
            ActivateTrailEmitters(PSC_TrailLeft, DAT_14810f328, DAT_14810f330, 0, FortAntelopeVehicleConfigs->NativeRecoveredInt32_0x53c);
        }
        if (PSC_TrailRight)
        {
            PSC_TrailRight->ActivateSystem(false);
            ActivateTrailEmitters(PSC_TrailRight, DAT_14810f338, DAT_14810f340, 0, FortAntelopeVehicleConfigs->NativeRecoveredInt32_0x53c);
        }
    }

    NativeRecoveredByte_0x1158 = (NativeRecoveredByte_0x1158 & 0xfe) | (bWantActive ? 1 : 0);
}
