// Exec 0x1429206b0 / inlined body 0x1419a75xx. Behavioral draft.
void AFortGameStateAthena::OnRep_SkyTubesDisabled()
{
    // Transition helper returns false for net mode 3 or missing world/material;
    // otherwise sets scalar material parameter SlipstreamsAlpha and returns true.
    // Tube helper updates +0x5c0 only on change, calls 0x142706d10, clears
    // +0x5c1 when enabled, then calls 0x142ad9950.
    // Actor iterator established using GetWorld and class accessor 0x142ad7c80.
    // Iterator filters pending-kill objects, level/world and class in engine code.
    for (auto* Tube : UnresolvedSkyTubeActorRange(GetWorld()))
        if (!UnresolvedTubeByteAt218(Tube))
            UnresolvedTubeSetEnabled_142706410(Tube, !bSkyTubesDisabled);
    if (bSkyTubesDisabled)
        UnresolvedSkyTubeTransition_1419af2c0(*this, 0.0f);
    else if (!bSkyTubesShuttingDown)
        UnresolvedFloatAt11f4(*this) = UnresolvedSkyTubeTransition_1419af2c0(*this, 1.0f) ? 0.0f : -1.0f;
}
