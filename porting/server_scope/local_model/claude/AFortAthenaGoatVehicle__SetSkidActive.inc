// CONFIDENCE: MEDIUM. Original exec 0x1427db700; native body 0x1418f7b40 (inlined). Parameter: bActive (bool).
// Same skid-trail logic as the antelope variant. Goat-specific members: flags byte at +0x10dc, trail systems at +0x1100
// (left) and +0x1108 (right), configs at +0x1118 with the speed field at +0x56c. The quality gate is the same.
const bool bWantActive = bActive && (GetVisualQualityLevel() < 2);
if (bWantActive != ((NativeRecoveredByte_0x10dc & 1) != 0))
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
            ActivateTrailEmitters(PSC_TrailLeft, DAT_1481100f0, DAT_1481100f8, 0, FortGoatVehicleConfigs->NativeRecoveredInt32_0x56c);
        }
        if (PSC_TrailRight)
        {
            PSC_TrailRight->ActivateSystem(false);
            ActivateTrailEmitters(PSC_TrailRight, DAT_148110100, DAT_148110108, 0, FortGoatVehicleConfigs->NativeRecoveredInt32_0x56c);
        }
    }

    NativeRecoveredByte_0x10dc = (NativeRecoveredByte_0x10dc & 0xfe) | (bWantActive ? 1 : 0);
}
