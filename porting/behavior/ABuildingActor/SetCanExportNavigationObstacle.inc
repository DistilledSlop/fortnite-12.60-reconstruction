// CONFIDENCE: HIGH
// Native 0x141bc4900; reflected exec 0x142738050.
if (bCanExportNavigationObstacle != InCanExportNavigationObstacle)
{
    bCanExportNavigationObstacle = InCanExportNavigationObstacle;
    UNavigationSystemV1::UpdateActorInNavOctree(*this);
}
