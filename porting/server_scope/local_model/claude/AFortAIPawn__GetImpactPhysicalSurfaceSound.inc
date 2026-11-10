// CONFIDENCE: MEDIUM. Classifies the impact into a physical surface and returns the matching impact sound from the pawn's per-surface sound table.
// Original address: 0x1427a4a30 (exec). Native body: 0x1422cb450 (table read at +0x1b60, 8-byte entries) with surface classifier 0x141ecd440 (GetSurfaceTypeFromHit).
FHitResult Impact; // struct parameter read from the script frame
const uint8 SurfaceType = GetSurfaceTypeFromHit(Impact);
return ImpactPhysicalSurfaceSounds[SurfaceType];
