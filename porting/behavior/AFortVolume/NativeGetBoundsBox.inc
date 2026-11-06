// CONFIDENCE: HIGH
// Native 0x141c756c0: get the live bounds once, then form both box corners.
FBoxSphereBounds Bounds;
NativeGetLocationBounds(Bounds);
return FBox(Bounds.Origin - Bounds.BoxExtent, Bounds.Origin + Bounds.BoxExtent);
